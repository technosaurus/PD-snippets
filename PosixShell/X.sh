# X11.sh - A pure BusyBox ash core-protocol abstraction layer

X11_NEXT_ID=4194304 # Starts our resource tracker at client base (0x00400000)
X11_FD=3            # The file descriptor bound to BusyBox nc

# Helper: Convert a decimal integer into a 4-byte Little Endian binary string
# Uses pure ash arithmetic to peel off bytes 0-3
pack_int32() {
    local val=$1
    local b0=$(( val & 255 ))
    local b1=$(( (val >> 8) & 255 ))
    local b2=$(( (val >> 16) & 255 ))
    local b3=$(( (val >> 24) & 255 ))
    printf "\\$(printf '%03o' $b0)\\$(printf '%03o' $b1)\\$(printf '%03o' $b2)\\$(printf '%03o' $b3)"
}

# Helper: Calculate padding for text strings to hit a 4-byte boundary
# Returns the absolute request length in dwords, and the trailing pad count
calc_padded_string_len() {
    local str_len=$1
    local pad=$(( (4 - (str_len % 4)) % 4 ))
    local total_bytes=$(( str_len + pad ))
    local dwords=$(( total_bytes / 4 ))
    
    # Export vars to the calling function frame safely
    X11_CALC_DWORDS=$dwords
    X11_CALC_PAD=$pad
}

x11_connect() {
    # Try xhost authorization first (works if we inherit user context)
    xhost +local: >/dev/null 2>&1

    echo "[X11.sh] Attempting connection cascade..."

    # STRATEGY 1: Zero External Tools (Pure Shell File Redirection)
    if exec 3<>/tmp/.X11-unix/X0 2>/dev/null; then
        echo " -> Mode: PURE SHELL (Native UNIX Socket)"
        return 0
    fi

    # STRATEGY 2: BusyBox Netcat via 'local:' syntax
    if exec 3<><(nc local:/tmp/.X11-unix/X0 2>/dev/null); then
        echo " -> Mode: BUSYBOX NC (local: syntax)"
        return 0
    fi

    # STRATEGY 3: Standard/Alternative Netcat via OpenBSD '-U' flag
    if exec 3<><(nc -U /tmp/.X11-unix/X0 2>/dev/null); then
        echo " -> Mode: BUSYBOX NC (-U UNIX Socket)"
        return 0
    fi

    # STRATEGY 4: Network Fallback (Assumes TCP port 6000 is open via localhost)
    # Tries to authorize the network port just in case
    xhost +localhost >/dev/null 2>&1
    if exec 3<><(nc 127.0.0.1 6000 2>/dev/null); then
        echo " -> Mode: NETWORK TCP (Port 6000 Fallback)"
        return 0
    fi

    echo "[X11.sh] FATAL: All connection vectors exhausted." >&2
    return 1
}

x11_init() {
    # 1. Establish the raw connection socket
    x11_connect
    
    # 2. Handshake (Little Endian, Client Major/Minor = 11.0)
    printf '\154\000\013\000\000\000\000\000\000\000\000\000' >&$X11_FD
    
    # 3. Read & Verify Accept Byte
    IFS= read -r -n 32 header <&$X11_FD
    if [ "$(printf '%d' "'${header~1}")" -ne 1 ]; then
        echo "X11.sh Error: Connection rejected or bad auth." >&2
        return 1
    fi
    #TODO Function to extract N digits to variable

    # Extract Resource ID Base (Bytes 12-15)
    base_b0=$(printf '%d' "'${header:12:1}")
    base_b1=$(printf '%d' "'${header:13:1}")
    base_b2=$(printf '%d' "'${header:14:1}")
    base_b3=$(printf '%d' "'${header:15:1}")
    X11_RESOURCE_BASE=$(( base_b0 | (base_b1 << 8) | (base_b2 << 16) | (base_b3 << 24) ))

    # Extract Resource ID Mask (Bytes 16-19)
    mask_b0=$(printf '%d' "'${header:16:1}")
    mask_b1=$(printf '%d' "'${header:17:1}")
    mask_b2=$(printf '%d' "'${header:18:1}")
    mask_b3=$(printf '%d' "'${header:19:1}")
    X11_RESOURCE_MASK=$(( mask_b0 | (mask_b1 << 8) | (mask_b2 << 16) | (mask_b3 << 24) ))

    # Reset our internal dynamic generator index to 0
    X11_CLIENT_ID_COUNTER=0
    # Flush out the dynamic server resource profile block safely
    IFS= read -r -n 512 -t 1 setup_block <&$X11_FD

}

x11_create_id() {
    # Increment our local counter
    X11_CLIENT_ID_COUNTER=$(( X11_CLIENT_ID_COUNTER + 1 ))
    
    # Securely mask and bind the counter to the server-allocated base
    local masked_counter=$(( X11_CLIENT_ID_COUNTER & X11_RESOURCE_MASK ))
    local final_id=$(( X11_RESOURCE_BASE | masked_counter ))
    
    # Return the Little-Endian 4-byte string ready for protocol packets
    pack_int32 $final_id
}

x11_create_window() {
    local wid="$1" parent="$2" x=$3 y=$4 w=$5 h=$6 bg_color=$7 event_mask=$8
    
    # Opcode 1, Depth=0, Total Request Length = 12 dwords
    printf '\x01\x00\x0c\x00' >&$X11_FD
    printf "%s%s" "$wid" "$parent" >&$X11_FD
    printf "%s%s" "$(pack_int32 $(( (y << 16) | x )))" "$(pack_int32 $(( (h << 16) | w )))" >&$X11_FD
    printf '\x00\x00\x01\x00\x00\x00\x00\x00' >&$X11_FD # Border=0, Class=InputOutput, Visual=0
    printf '\x0a\x00\x00\x00' >&$X11_FD               # ValueMask = CWBackPixel(2) | CWEventMask(8)
    printf "%s%s" "$(pack_int32 $bg_color)" "$(pack_int32 $event_mask)" >&$X11_FD
}

x11_map_window() {
    # Opcode 8, Length = 2 dwords
    printf '\x08\x00\x02\x00' >&$X11_FD
    printf "%s" "$1" >&$X11_FD
}

x11_draw_text() {
    local wid="$1" gcid="$2" x=$3 y=$4 text="$5"
    local t_len=${#text}
    
    # Use our arithmetic helper to determine padding metrics dynamically
    calc_padded_string_len $t_len
    local req_dwords=$(( 4 + X11_CALC_DWORDS )) # Base header is 4 dwords
    
    # Opcode 76 (ImageText8), single-byte string size tracking
    printf "\\$(printf '%03o' 76)\\$(printf '%03o' $t_len)\\$(printf '%03o' $((req_dwords & 255)))\\$(printf '%03o' $((req_dwords >> 8)))" >&$X11_FD
    printf "%s%s" "$wid" "$gcid" >&$X11_FD
    printf "%s%s" "$(pack_int32 $x)" "$(pack_int32 $y)" >&$X11_FD
    printf "%s" "$text" >&$X11_FD
    
    # Append the calculated null padding bytes seamlessly
    [ $X11_CALC_PAD -gt 0 ] && printf "%0${X11_CALC_PAD}d" 0 | tr '0' '\000' >&$X11_FD
}


x11_get_atom() {
    local atom_name="$1"
    local a_len=${#atom_name}
    
    calc_padded_string_len $a_len
    local req_dwords=$(( 2 + X11_CALC_DWORDS )) # Header is 2 dwords
    
    # Request: InternAtom (Opcode 16), only_if_exists=1 (true)
    printf "\\$(printf '%03o' 16)\x01\\$(printf '%03o' $((req_dwords & 255)))\\$(printf '%03o' $((req_dwords >> 8)))" >&$X11_FD
    printf "\\$(printf '%03o' $((a_len & 255)))\\$(printf '%03o' $((a_len >> 8)))\x00\x00" >&$X11_FD
    printf "%s" "$atom_name" >&$X11_FD
    [ $X11_CALC_PAD -gt 0 ] && printf "%0${X11_CALC_PAD}d" 0 | tr '0' '\000' >&$X11_FD
    
    # Read response structure from X server (Generic Reply always begins with 0x01)
    IFS= read -r -n 32 reply <&$X11_FD
    
    # Bytes 8-11 contain the 32-bit Atom identifier value
    local b0=$(printf '%d' "'$(printf '%s' "$reply" | cut -c 9)")
    local b1=$(printf '%d' "'$(printf '%s' "$reply" | cut -c 10)")
    local b2=$(printf '%d' "'$(printf '%s' "$reply" | cut -c 11)")
    local b3=$(printf '%d' "'$(printf '%s' "$reply" | cut -c 12)")
    
    # Reassemble Little Endian integer back into shell value
    local atom_id=$(( b0 | (b1 << 8) | (b2 << 16) | (b3 << 24) ))
    echo "$atom_id"
}


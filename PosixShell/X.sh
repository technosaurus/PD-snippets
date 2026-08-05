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

x11_init() {
    # 1. Establish the raw connection socket
    exec 3<><(busybox nc 127.0.0.1 6000)
    
    # 2. Handshake (Little Endian, Client Major/Minor = 11.0)
    printf '\154\000\013\000\000\000\000\000\000\000\000\000' >&$X11_FD
    
    # 3. Read & Verify Accept Byte
    IFS= read -r -n 8 header <&$X11_FD
    if [ "$(printf '%d' "'${header~1}")" -ne 1 ]; then
        echo "X11.sh Error: Connection rejected or bad auth." >&2
        return 1
    fi
    
    # Flush out the dynamic server resource profile block safely
    IFS= read -r -n 512 -t 1 skipped <&$X11_FD
}

x11_create_id() {
    X11_NEXT_ID=$((X11_NEXT_ID + 1))
    pack_int32 $X11_NEXT_ID
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

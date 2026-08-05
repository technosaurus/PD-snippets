# X11.sh - A pure BusyBox ash core-protocol abstraction layer

X11_NEXT_ID=4194304 # Starts our resource tracker at client base (0x00400000)
X11_FD=3            # The file descriptor bound to BusyBox nc
X11_CLIENT_ID_COUNTER=0 # Set our internal dynamic generator index to 0

# Helper: Convert a shell integer into a 4-byte Little Endian binary string
# Highlighter friendly, zero subshells, single printf execution
pack_int32() {
    printf "\\x%02x\\x%02x\\x%02x\\x%02x" \
        $(( $1 & 255 )) $(( ($1 >> 8) & 255 )) \
        $(( ($1 >> 16) & 255 )) $(( ($1 >> 24) & 255 ))
}

# Helper: Safely unpack integers from a binary block by converting characters
# to their exact ASCII index without breaking code highlighters
x11_unpack_int() {
    local str="$1" offset=$2 bytes=$3
    local val=0 i=0 shift=0 char_val=0

    while [ $i -lt $bytes ]; do
        # We use a completely clean format statement to get the decimal ASCII byte
        char_val=$(printf "%d" "\"${str:$((offset + i)):1}")
        
        val=$(( val | (char_val << shift) ))
        shift=$(( shift + 8 ))
        i=$(( i + 1 ))
    done
    
    echo "$val"
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

# Centering Calculation Engine
# Usage: calculate_center TOTAL_CANVAS_SIZE OBJECT_SIZE
calculate_center() {
    local total=$1
    local object=$2
    echo $(( (total - object) / 2 ))
}

#  Dynamic Window Centering Function
# Spawns a window perfectly dead-center on the monitor
x11_create_centered_window() {
    local w=$1 h=$2 bg_color=$3 event_mask=$4
    
    # Calculate perfect coordinates using our parsed monitor bounds
    local target_x=$(calculate_center $X11_SCREEN_WIDTH $w)
    local target_y=$(calculate_center $X11_SCREEN_HEIGHT $h)
    
    # Generate an ID and call our core window allocator
    local wid=$(x11_create_id)
    x11_create_window "$wid" "$(pack_int32 $X11_ROOT_WINDOW_ID)" $target_x $target_y $w $h $bg_color $event_mask
    
    # Return the raw window ID token back to the script variable
    echo "$wid"
}

#  Dynamic Text Centering Function
# Draws a string perfectly centered horizontally within a window
x11_draw_centered_text() {
    local wid="$1" gcid="$2" win_w=$3 dest_y=$4 text="$5"
    
    # Core X11 fallback font character metrics (Assuming standard 6px width per char)
    local char_w=6 
    
    # Calculate the total string footprint in pixels
    local text_w=$(( ${#text} * char_w ))
    
    # Calculate the starting X coordinate relative to the window boundary
    local start_x=$(calculate_center $win_w $text_w)
    
    # Prevent negative boundaries if text overflows the canvas
    [ $start_x -lt 0 ] && start_x=0
    
    # Stream the text with calculated alignment coordinates
    x11_draw_text "$wid" "$gcid" $start_x $dest_y "$text"
}

x11_draw_zpixmap() {
    local wid="$1" gcid="$2" w=$3 h=$4 dest_x=$5 dest_y=$6
    
    local bytes_per_row=$(( w * 4 ))
    local req_dwords=$(( 6 + ((bytes_per_row * h) / 4) ))

    # Opcode 72 (PutImage), Format 2 (ZPixmap)
    # Complete header pushed down the socket descriptor in one single shot
    printf "\\x48\\x02\\x%02x\\x%02x%s%s\\x%02x\\x%02x\\x%02x\\x%02x\\x%02x\\x%02x\\x%02x\\x%02x\\x00\\x18\\x00\\x00" \
        $((req_dwords & 255)) $((req_dwords >> 8)) \
        "$wid" "$gcid" \
        $((w & 255)) $((w >> 8)) $((h & 255)) $((h >> 8)) \
        $((dest_x & 255)) $((dest_x >> 8)) $((dest_y & 255)) $((dest_y >> 8)) >&$X11_FD
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
    # Establish the raw connection socket
    x11_connect
    
    # Handshake (Little Endian, Client Major/Minor = 11.0)
    printf '\154\000\013\000\000\000\000\000\000\000\000\000' >&$X11_FD

    # Read the complete 32-byte header block
    IFS= read -r -n 32 setup_block <&$X11_FD
    
    # Verify the success status (Byte offset 0, 1 byte long)
    if [ "$(x11_unpack_int "$setup_block" 0 1)" -ne 1 ]; then
        echo "Connection refused or authentication failed." >&2
        exit 1
    fi
    
    # Extract Client Resource configurations cleanly
    X11_RESOURCE_BASE=$(x11_unpack_int "$setup_block" 12 4)
    X11_RESOURCE_MASK=$(x11_unpack_int "$setup_block" 16 4)
    
    # Unpack lengths from the 32-byte header to locate our upcoming offsets
    local vendor_len=$(x11_unpack_int "$setup_block" 16 2)  # Byte 16, 2 bytes long
    local num_formats=$(x11_unpack_int "$setup_block" 21 1) # Byte 21, 1 byte long
    
    # Account for the 4-byte padding on the vendor string length
    local vendor_pad=$(( (4 - (vendor_len % 4)) % 4 ))
    local vendor_total_bytes=$(( vendor_len + vendor_pad ))
    
    # Calculate formats block size (8 bytes per format entry)
    local formats_total_bytes=$(( num_formats * 8 ))
    
    # Compute the exact index where the Screen Structure starts
    local screen_start=$(( vendor_total_bytes + formats_total_bytes ))

    # Calculate remaining payload sizes seamlessly (Byte offset 6, 2 bytes long)
    remaining_dwords=$(x11_unpack_int "$setup_block" 6 2)
    remaining_bytes=$(( remaining_dwords * 4 ))
    
    # Flush and load the variable payload block
    IFS= read -r -n $remaining_bytes dynamic_payload <&$X11_FD
    
    # Extract the 40-byte Screen parameters block using pure ash substrings!
    # We slice 40 bytes out of our raw dynamic payload variable natively
    screen="${dynamic_payload:$screen_start:40}"
    X11_ROOT_WINDOW_ID=$(x11_unpack_int "$screen" 0 4)
    X11_DEFAULT_COLORMAP=$(x11_unpack_int "$screen" 4 4) # Offset 4, 4 Bytes
    X11_WHITE_PIXEL=$(x11_unpack_int "$screen" 8 4)      # Offset 8, 4 Bytes
    X11_BLACK_PIXEL=$(x11_unpack_int "$screen" 12 4)     # Offset 12, 4 Bytes

    X11_SCREEN_WIDTH=$(x11_unpack_int "$screen" 24 2)
    X11_SCREEN_HEIGHT=$(x11_unpack_int "$screen" 26 2)

}

x11_create_text_gc() {
    local gcid="$1" target_wid="$2" fg_pixel=$3 bg_pixel=$4
    
    # Opcode 55 (CreateGC), Length = 6 dwords (24 bytes)
    printf '\x37\x00\x06\x00' >&$X11_FD
    printf "%s%s" "$gcid" "$target_wid" >&$X11_FD
    
    # Value Mask = GCForeground (0x04) | GCBackground (0x08) = 12 (0x0c)
    printf '\x0c\x00\x00\x00' >&$X11_FD
    
    # Stream the dynamic 4-byte pixel parameters
    printf "%s%s" "$(pack_int32 $fg_pixel)" "$(pack_int32 $bg_pixel)" >&$X11_FD
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

# Optimized Atom Fetcher using native ${var:offset:len} matching
x11_get_atom() {
    local atom_name="$1"
    local a_len=${#atom_name}
    local pad=$(( (4 - (a_len % 4)) % 4 ))
    local req_dwords=$(( 2 + ((a_len + pad) / 4) ))

    # Send InternAtom header and configuration payload in one single shot
    printf "\\x10\\x01\\x%02x\\x%02x\\x%02x\\x%02x\\x00\\x00%s" \
        $((req_dwords & 255)) $((req_dwords >> 8)) \
        $((a_len & 255)) $((a_len >> 8)) "$atom_name" >&$X11_FD
    [ $pad -gt 0 ] && printf "%0${pad}d" 0 | busybox tr '0' '\000' >&$X11_FD
    
    IFS= read -r -n 32 reply <&$X11_FD
    
    # Native Substring extraction instead of 'cut'
    local atom=$(x11_unpack_int "$reply"  4)
    echo "$atom"
}

# Tell the Window Manager to pass us close messages instead of killing us
x11_enable_graceful_close() {
    local wid="$1"
    
    # Resolve the protocol atom indices dynamically
    ATOM_WM_PROTOCOLS=$(x11_get_atom "WM_PROTOCOLS")
    ATOM_WM_DELETE_WINDOW=$(x11_get_atom "WM_DELETE_WINDOW")
    
    # Request: ChangeProperty (Opcode 18), Format 32 (32-bit integers)
    # Mode 0 (Replace), Length = 7 dwords
    printf '\x12\x00\x07\x00' >&$X11_FD
    printf "%s" "$wid" >&$X11_FD
    printf "%s" "$(pack_int32 $ATOM_WM_PROTOCOLS)" >&$X11_FD
    printf "%s" "$(pack_int32 4)" >&$X11_FD # Type atom (4 = ATOM)
    printf '\x20\x00\x00\x00' >&$X11_FD    # Format=32, Pad bytes
    printf '\x01\x00\x00\x00' >&$X11_FD    # Data Length = 1 entry
    printf "%s" "$(pack_int32 $ATOM_WM_DELETE_WINDOW)" >&$X11_FD
}

example(){
# Initialize connection and unpack global screen data dynamically
x11_init || exit 1

MY_GC="$(x11_create_id)"
WIN_WIDTH=400
WIN_HEIGHT=150

# 1. Spawn the window perfectly centered on your screen
# Background: White, EventMask: KeyPress(1)
MY_WINDOW=$(x11_create_centered_window $WIN_WIDTH $WIN_HEIGHT 16777215 1)
x11_map_window "$MY_WINDOW"

# Initialize our standard black drawing tool text graphics context
printf '\x37\x00\x05\x00' >&3; printf "$MY_GC" >&3; printf "$MY_WINDOW" >&3
printf '\x04\x00\x00\x00' >&3; printf '\x00\x00\x00\x00' >&3

# 2. Draw multiple text lines, perfectly centered inside the window
x11_draw_centered_text "$MY_WINDOW" "$MY_GC" $WIN_WIDTH 50 "--- Welcome to X11.sh ---"
x11_draw_centered_text "$MY_WINDOW" "$MY_GC" $WIN_WIDTH 80 "This layout is completely symmetrical."
x11_draw_centered_text "$MY_WINDOW" "$MY_GC" $WIN_WIDTH 110 "Press any key to exit."

# Initialize standard black text on a white canvas background
MY_GC=$(x11_create_id)
x11_create_text_gc "$MY_GC" "$MY_WINDOW" $X11_BLACK_PIXEL $X11_WHITE_PIXEL

# Draw text (will now use the exact system-accurate black pixel token)
x11_draw_centered_text "$MY_WINDOW" "$MY_GC" $WIN_WIDTH 50 "System Accurate Colors!"

# Block execution inside the interactive listener loop
while true; do
    if IFS= read -r -n 32 event <&3; then
        evt_type_hex=$(printf '%02x' "'${event:0:1}")
        [ "$evt_type_hex" = "02" ] && break # Break loop on KeyPress
    fi
done

exec 3>&-
}














#!/bin/sh
# Pure POSIX shell script optimized for legacy BusyBox (No HTTPS/SSL support)
# Uses http:// instead of https:// and zero external string binaries

RAW_STRING="Hello World"
SRC_LANG="en"
TARGET_LANG="es"

# Minimalist POSIX-only space to '+' encoder (avoids xxd/hexdump/sed)
ENCODED_STRING=""
case "$RAW_STRING" in
    *" "*) ENCODED_STRING=$(printf '%s' "$RAW_STRING" | tr ' ' '+') ;;
    *) ENCODED_STRING="$RAW_STRING" ;;
esac

# Formulate unencrypted HTTP request targeting the Google Extension endpoint
# -q = quiet, -O - = output directly to stdout
URL="http://googleapis.com{SRC_LANG}&tl=${TARGET_LANG}&dt=t&q=${ENCODED_STRING}"
RAW_RESPONSE=$(busybox wget -q -O - "$URL")

# --- THE TECHNOSAURUS EXTRACTION METHOD ---
# Google returns arrays looking like: [[["Hola Mundo","Hello World",null,null,1]]]
# We drop everything up to the first open quote, then chop everything after the closing quote.

# 1. Strip the leading JSON brackets and the opening quote: [[["
STRIPPED_FRONT="${RAW_RESPONSE#*[[[\""

# 2. Chop off the trailing quote and everything following it: ","Hello World"...
TRANSLATED_TEXT="${STRIPPED_FRONT%%\",\"*}"

echo "Original text:   $RAW_STRING"
echo "Translated text: $TRANSLATED_TEXT"




#!/bin/sh
# Minimalist script parsing ${TTx:-"text"} structure via BusyBox logic
# Automatically pipes strings to Google Translate via raw unencrypted HTTP

TARGET_SCRIPT="$1"
TARGET_LANG="$2" # e.g., "es", "de", "fr"

if [ -z "$TARGET_SCRIPT" ] || [ -z "$TARGET_LANG" ]; then
    echo "Usage: $0 <script_file> <language_code>"
    echo "Example: $0 network-wizard.sh es"
    exit 1
fi

OUTPUT_FILE="strings.${TARGET_LANG}"
echo "# Automated Translation Map for ${TARGET_LANG}" > "$OUTPUT_FILE"

echo "Scraping variables and fetching translations..."

# 1. Scrape lines matching the ${TT<num>:-<text>} pattern
# The regex isolates the exact TT index and the text within the brackets
grep -o '\${TT[0-9]*:[^}]*}' "$TARGET_SCRIPT" | sort -u | while read -r MATCH
do
    # Example MATCH: ${TT1:-Thanks}
    
    # 2. Use pure shell built-ins to isolate the variable key and the default text
    # Strip the leading '${' and trailing '}'
    CLEAN="${MATCH#\${}"
    CLEAN="${CLEAN%\}}"
    
    # Extract the token index (everything before the ':-')
    TT_VAR="${CLEAN%%:-*}"
    
    # Extract the master string text value (everything after ':-')
    RAW_STRING="${CLEAN#*:-}"
    
    # Optional cleanup: Strip quotes if the developer wrapped them inside the fallback syntax
    RAW_STRING=$(printf '%s' "$RAW_STRING" | tr -d '"'\')

    if [ -n "$RAW_STRING" ] && [ -n "$TT_VAR" ]; then
        # 3. Micro URL-encoder for BusyBox wget
        ENCODED_STRING=""
        case "$RAW_STRING" in
            *" "*) ENCODED_STRING=$(printf '%s' "$RAW_STRING" | tr ' ' '+') ;;
            *) ENCODED_STRING="$RAW_STRING" ;;
        esac

        # 4. Pull payload over port 80 unencrypted HTTP endpoint (bypasses HTTPS/SSL binaries)
        URL="http://googleapis.com{TARGET_LANG}&dt=t&q=${ENCODED_STRING}"
        RAW_RESPONSE=$(busybox wget -q -O - "$URL")

        # 5. Extract translated string directly out of Google's nested JSON bracket array
        STRIPPED_FRONT="${RAW_RESPONSE#*[[[\""
        TRANSLATED_TEXT="${STRIPPED_FRONT%%\",\"*}"

        # If fallback fails, preserve the original English text string
        if [ -z "$TRANSLATED_TEXT" ] || [ "$TRANSLATED_TEXT" = "$RAW_RESPONSE" ]; then
            TRANSLATED_TEXT="$RAW_STRING"
        fi

        # 6. Append directly into your exact requested output format
        echo "${TT_VAR}=\"${TRANSLATED_TEXT}\"" >> "$OUTPUT_FILE"
        echo "Processed: $TT_VAR -> $TRANSLATED_TEXT"
        
        # Short pause to reduce IP flagging risks on Google's endpoint
        sleep 1
    fi
done

echo "Done! File saved as: $OUTPUT_FILE"



#!/bin/sh
# Minimalist PoC for xdg-edit key parsing to share with upstream maintainers
FILE_MIME=$(file -b --mime-type "$1")
CONFIG="${XDG_CONFIG_HOME:-$HOME/.config}/mimeapps.edit"

# 1. Check user explicit defaults first
HANDLER=$(grep "^${FILE_MIME}=" "$CONFIG" | cut -d'=' -f2)

# 2. Dynamic fallback scanning for the array key across standard paths
if [ -z "$HANDLER" ]; then
    for dir in ~/.local/share/applications /usr/share/applications; do
        for dt in "$dir"/*.desktop; do
            if grep -q "^X-Xlibre-MimeEdit=.*;${FILE_MIME};" "$dt" 2>/dev/null; then
                EXEC_CMD=$(grep "^Exec=" "$dt" | head -n1 | cut -d'=' -f2- | sed 's/%[fFuU]//g')
                exec $EXEC_CMD "$1"
            fi
        done
    done
fi

applet_file_browser() {
    TARGET_DIR="${1:-$HOME}"
    
    # Normalize trailing slash
    case "$TARGET_DIR" in
        */) ;;
        *) TARGET_DIR="${TARGET_DIR}/" ;;
    esac

    # JWM requires an enveloping root XML tag for dynamic menus
    echo "<JWM>"
    
    # 1. Provide an administrative escape hatch terminal link
    echo "  <Program label=\"[ Open Terminal Here ]\" icon=\"terminal.png\">cd \"$TARGET_DIR\" && exec rxvt</Program>"
    echo "  <Separator/>"

    # 2. Loop through files and directories inside the current target path
    for entry in "${TARGET_DIR}"*; do
        [ -e "$entry" ] || continue
        NAME="${entry##*/}"

        if [ -d "$entry" ]; then
            # --- DIRECTORY HANDLING ---
            # Recursively spawns a brand new, hot-cached dynamic sub-menu on hover
            echo "  <Menu label=\"${NAME}/\" icon=\"folder.png\">"
            echo "    <Dynamic>exec:bashbox file-browser \"$entry\"</Dynamic>"
            echo "  </Menu>"
        else
            # --- REGULAR FILE HANDLING ---
            # Automatically detect file MIME type via BusyBox / Toybox
            MIME_TYPE=$(file -b --mime-type "$entry" 2>/dev/null)
            [ -z "$MIME_TYPE" ] && MIME_TYPE="application/octet-stream"

            # Dynamic Icon Assignment based on basic MIME matching structures
            case "$MIME_TYPE" in
                text/*|application/json|application/xml) ICON="text-x-generic.png" ;;
                image/*)                                 ICON="image-x-generic.png" ;;
                audio/*|video/*)                         ICON="audio-x-generic.png" ;;
                application/x-executable|application/x-sharedlib) ICON="application-x-executable.png" ;;
                *)                                       ICON="application-x-octet-stream.png" ;;
            esac

            # Encapsulate the file inside its own nested contextual action menu block
            echo "  <Menu label=\"$NAME\" icon=\"$ICON\">"
            
            # Action 1: Open Option (Routes straight to your xdg-open applet)
            echo "    <Program label=\"[ Open ]\" icon=\"eye.png\">bashbox xdg-open \"$entry\"</Program>"
            
            # Action 2: Edit Option (Routes straight to your custom xdg-edit applet)
            echo "    <Program label=\"[ Edit ]\" icon=\"pencil.png\">bashbox xdg-edit \"$entry\"</Program>"
            
            echo "    <Separator/>"
            
            # Action 3: Secure Delete Option (Generates a nested confirmation safety sub-menu)
            echo "    <Menu label=\"[ Delete... ]\" icon=\"edit-delete.png\">"
            echo "      <Program label=\"⚠️ NO, Cancel\" icon=\"gtk-cancel.png\">true</Program>"
            echo "      <Program label=\"🔥 YES, Delete Permanently\" icon=\"gtk-ok.png\">rm -f \"$entry\"</Program>"
            echo "    </Menu>"
            
            echo "  </Menu>"
        fi
    done

    echo "</JWM>"
}

#don't waste time and space linking every file in a directory when bind mount is appropriate
#sudo mount --bind /usr/share/applications /home/yourusername/.config/applications
#however bind mount are full write or read only, to make the source read only but keep write capacity
#sudo mount -t overlay overlay -o lowerdir=/usr/share/applications,upperdir=$HOME/.overlays/apps_upper,workdir=$HOME/.overlays/apps_upper \
#  $HOME/.config/applications
  
coproc() {
    local name
    
    # Check if the first argument is a runnable command/applet
    if command -v "$1" >/dev/null 2>&1; then
        # It's a command! Fallback to default Bash naming behavior
        name="COPROC"
    else
        # It's not a command, treat it as the custom variable name
        name="$1"
        shift
    fi

    if [ -z "$name" ] || [ $# -eq 0 ]; then
        echo "Usage: coproc [NAME] COMMAND [ARGS...]" >&2
        return 1
    fi

    local pipe_in="/tmp/coproc_${$}_${name}_in"
    local pipe_out="/tmp/coproc_${$}_${name}_out"
    
    rm -f "$pipe_in" "$pipe_out"
    mkfifo "$pipe_in" "$pipe_out"

    "$@" < "$pipe_in" > "$pipe_out" &
    
    eval "${name}_PID=$!"
    eval "${name}_IN=\"\$pipe_in\""
    eval "${name}_OUT=\"\$pipe_out\""
}

coproc_close() {
    local name="${1:-COPROC}"
    eval "rm -f \"\${${name}_IN}\" \"\${${name}_OUT}\""
    eval "unset ${name}_PID ${name}_IN ${name}_OUT"
}

# Core Shim Library: Non-blocking Asynchronous Processes

async_spawn() {
    # Usage: async_spawn NAME COMMAND [ARGS...]
    local name="$1"
    shift

    if [ -z "$name" ] || [ $# -eq 0 ]; then
        echo "Usage: async_spawn NAME COMMAND [ARGS...]" >&2
        return 1
    fi

    # Create temporary non-blocking descriptors via files/pipes
    local pipe_in="/tmp/async_${$}_${name}_in"
    local pipe_out="/tmp/async_${$}_${name}_out"
    
    rm -f "$pipe_in" "$pipe_out"
    mkfifo "$pipe_in" "$pipe_out"

    # Background executor block compatible across ash/dash/hush/toysh
    "$@" < "$pipe_in" > "$pipe_out" 2>&1 &
    
    eval "${name}_PID=$!"
    eval "${name}_IN=\"\$pipe_in\""
    eval "${name}_OUT=\"\$pipe_out\""
}

async_poll() {
    # Usage: async_poll NAME
    # Returns 0 if data is ready to read, 1 if no data is present (non-blocking)
    local name="${1:-ASYNC}"
    local pipe_out
    eval "pipe_out=\${${name}_OUT}"

    # Use standard POSIX test checking if file size is greater than 0 bytes
    if [ -s "$pipe_out" ]; then
        return 0
    fi
    return 1
}

async_read() {
    # Usage: async_read NAME VARNAME
    # Safely drains buffer only if data has landed
    local name="$1"
    local out_var="$2"
    local pipe_out
    eval "pipe_out=\${${name}_OUT}"

    if [ -s "$pipe_out" ]; then
        # Read the raw data buffer and remove it from the pipe queue safely
        local content
        read -r content < "$pipe_out" || [ -n "$content" ]
        eval "$out_var=\"\$content\""
        return 0
    fi
    return 1
}

async_close() {
    local name="${1:-ASYNC}"
    local pid
    eval "pid=\${${name}_PID}"

    # Standard POSIX-safe process teardown
    if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
        kill -15 "$pid" 2>/dev/null
    fi

    eval "rm -f \"\${${name}_IN}\" \"\${${name}_OUT}\""
    eval "unset ${name}_PID ${name}_IN ${name}_OUT"
}

serialize_variables_to_json() {
    # Check if we are running in Bash
    if [ -n "$BASH_VERSION" ]; then
        # =====================================================================
        # BASH MODE (Supports Arrays & Advanced Builtins)
        # =====================================================================
        local var attrs val element first_item=true
        
        printf '{'
        
        # compgen -v is a Bash builtin that lists all variable names
        for var in $(compgen -v); do
            # Filter out standard system noise to keep the JSON manageable
            case "$var" in
                BASH*|COMP*|DIRSTACK|GROUPS|FUNCNAME|HIST*|IFS|PIPESTATUS|POSIX*|SHELL*|UID) continue ;;
            esac

            # Use declare -p metadata to identify the type
            attrs=$(declare -p "$var" 2>/dev/null)
            
            # Print a comma if this isn't our first item
            if [ "$first_item" = true ]; then first_item=false; else printf ','; fi
            
            if [[ "$attrs" == *"declare -a"* || "$attrs" == *"declare -A"* ]]; then
                # Handle Array: Use a modern Bash nameref to loop through keys/values
                local -n array_ref="$var"
                local dynamic_comma=""
                
                printf '"%s":{"type":"array","value":[' "$var"
                for element in "${array_ref[@]}"; do
                    # Escape inner quotes/backslashes inside the array element using pure Bash builtins
                    val="${element//\\/\\\\}"
                    val="${val//\"/\\\"}"
                    val="${val//$'\n'/\\n}" # Handles line breaks if any
                    
                    printf '%s"%s"' "$dynamic_comma" "$val"
                    dynamic_comma=","
                done
                printf ']}'
            else
                # Handle Scalar string
                val="${!var}"
                val="${val//\\/\\\\}"
                val="${val//\"/\\\"}"
                val="${val//$'\n'/\\n}"
                
                printf '"%s":{"type":"scalar","value":"%s"}' "$var" "$val"
            fi
        done
        printf '}\n'

    else
        # =====================================================================
        # POSIX SH MODE (Scalars Only, Pure Builtins)
        # =====================================================================
        # We must rely on 'set' because compgen doesn't exist.
        # To avoid external sed, we stream the output of 'set' line-by-line 
        # and parse it using standard shell parameter expansion.
        
        printf '{'
        first_item=true
        
        set | while IFS= read -r line; do
            # Extract just the variable name (everything before the first '=')
            var="${line%%=*}"
            
            # Skip invalid lines (like functions printed by some POSIX environments)
            # or massive built-in paths
            case "$var" in
                ""|*" "*|IFS|PATH|PWD|HOME|PPID|SHELL|OPTIND|PS1|PS2|PS4) continue ;;
            esac
            
            # Fetch value using POSIX-compliant dynamic evaluation
            eval "val=\"\$$var\""
            
            # Escape strings purely using POSIX string replacement loops if needed,
            # but standard parameter expansions work for the basics:
            # (Note: POSIX sh doesn't support the ${var//search/replace} syntax, 
            # so we keep it simple or do a character-by-character check if required.
            # Assuming basic string content for this minimal engine)
            
            if [ "$first_item" = true ]; then first_item=false; else printf ','; fi
            printf '"%s":{"type":"scalar","value":"%s"}' "$var" "$val"
        done
        printf '}\n'
    fi
}

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


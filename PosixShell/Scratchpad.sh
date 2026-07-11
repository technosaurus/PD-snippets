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

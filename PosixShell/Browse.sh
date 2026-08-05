#!/bin/sh

# ==============================================================================
# GLOBAL STATE & INITIALIZATION
# ==============================================================================
CURRENT_URL="http://127.0.0"
COOKIE_FILE="/tmp/tazpanel_cookie"
USE_COOKIES=1
ACTION_MODE="GET"

TOTAL_FIELDS=0
CUR_FIELD=0 # 0 = Address bar
URL_BAR_VALUE="$CURRENT_URL"
ESC=$(printf '\033')

# Mock Initial HTML payload (Sourced to mimic a live TazPanel CGI application)
html="<h1>Network Settings</h1><label>Hostname</label><input type='text' name='hostname' value='slitaz-box'><label>Root Password</label><input type='password' name='rootpass' value='secret123'><label>Interface Mode</label><select name='iface'><option>Static_IP</option><option>DHCP_Client</option><option>PPPoE_Dialup</option></select><label>Config Summary</label><textarea name='summary'>Default fallback text description.</textarea><label>Upload Certificate</label><input type='file' name='certfile'><a href='advanced.cgi'>View Advanced Logs</a>"

# Helper macros to abstract the messy dynamic global variables
get_field() { eval "echo \"\$FIELD_${1}_$2\""; }
set_field() { eval "FIELD_${1}_$2=\"\$3\""; }

# ==============================================================================
# 1. CORE NETWORKING ENGINE
# ==============================================================================
request_page() {
    local method="$1" url="$2" payload="$3"
    local wget_opts="-q -O-"
    
    [ "$USE_COOKIES" -eq 1 ] && wget_opts="$wget_opts --save-cookies=$COOKIE_FILE --load-cookies=$COOKIE_FILE --keep-session-cookies"
    
    if [ "$method" = "POST" ] && [ -n "$payload" ]; then
        # Check if the payload is an absolute local file descriptor path
        if [ "${payload##/*}" = "" ] && [ -f "$payload" ]; then
            wget $wget_opts --post-file="$payload" "$url"
        else
            wget $wget_opts --post-data="$payload" "$url"
        fi
    else
        wget $wget_opts "$url"
    fi
}

resolve_url() {
    local target="$1"
    case "$target" in
        http://*|https://*) echo "$target" ;;
        /*)
            local proto="${CURRENT_URL%%://*}" tmp="${CURRENT_URL#*://}" domain="${tmp%%/*}"
            echo "${proto}://${domain}${target}"
            ;;
        *)
            echo "$(dirname "$CURRENT_URL")/${target#./}"
            ;;
    esac
}

# ==============================================================================
# 2. HTML TOKENIZER & ELEMENT MAPPER
# ==============================================================================
add_field_to_state() {
    TOTAL_FIELDS=$((TOTAL_FIELDS + 1))
    set_field "TYPE" "$TOTAL_FIELDS" "$1"
    set_field "NAME" "$TOTAL_FIELDS" "$2"
    set_field "VALUE" "$TOTAL_FIELDS" "$3"
    set_field "LABEL" "$TOTAL_FIELDS" "$4"
}

# ==============================================================================
# TAG-SPECIFIC SUBSURFACE HTML PARSERS
# ==============================================================================

parse_input_tag() {
    local tag_body="$1" local lbl="$2"
    TOTAL_FIELDS=$((TOTAL_FIELDS + 1))
    
    # Extract structural attributes using positional parameter string tricks
    local tmp_name="${tag_body#*name=\'}" ; [ "$tmp_name" = "$tag_body" ] && tmp_name="${tag_body#*name=\"}"
    local tmp_val="${tag_body#*value=\'}"   ; [ "$tmp_val" = "$tag_body" ]  && tmp_val="${tag_body#*value=\"}"
    local f_name="${tmp_name%%['\"]*}"     ; local f_val="${tmp_val%%['\"]*}"
    
    # Map element type to appropriate visual and interaction profiles
    case "$tag_body" in
        *type=\'password\'*|*type=\"password\"*) 
            add_field_to_state "password" "$f_name" "$f_val" "${lbl:-Password}" 
            ;;
        *type=\'submit\'*|*type=\"submit\"*)     
            add_field_to_state "submit" "$f_name" "$f_val" "[ACTION]" 
            ;;
        *type=\'file\'*|*type=\"file\"*)         
            add_field_to_state "file" "$f_name" "" "${lbl:-Select File}" 
            ;;
        *)                                       
            add_field_to_state "text" "$f_name" "$f_val" "${lbl:-Text Field}" 
            ;;
    esac
}

parse_textarea_tag() {
    local tag_body="$1" local full_stream="$2" local lbl="$3"
    TOTAL_FIELDS=$((TOTAL_FIELDS + 1))
    
    local tmp_name="${tag_body#*name=\'}" ; [ "$tmp_name" = "$tag_body" ] && tmp_name="${tag_body#*name=\"}"
    local f_name="${tmp_name%%['\"]*}"
    
    # Textareas hold text inside tags; harvest it out of the stream ahead
    local inner_val="${full_stream#*>}"
    inner_val="${inner_val%%<\/textarea*}" ; inner_val="${inner_val%%<\/TEXTAREA*}"
    
    add_field_to_state "text" "$f_name" "$inner_val" "${lbl:-Textarea}"
}

parse_select_tag() {
    local tag_body="$1" local full_stream="$2" local lbl="$3"
    TOTAL_FIELDS=$((TOTAL_FIELDS + 1))
    
    local tmp_name="${tag_body#*name=\'}" ; [ "$tmp_name" = "$tag_body" ] && tmp_name="${tag_body#*name=\"}"
    local f_name="${tmp_name%%['\"]*}"
    
    # Isolate the nested options data stream
    local opt_stream="${full_stream#*<select}" ; [ "$opt_stream" = "$full_stream" ] && opt_stream="${full_stream#*<SELECT}"
    opt_stream="${opt_stream%%<\/select*}" ; opt_stream="${opt_stream%%<\/SELECT*}"
    
    local option_list=""
    while [ -n "$opt_stream" ]; do
        case "$opt_stream" in
            *\<option\>*|\*\<OPTION\>*)
                local o_tmp="${opt_stream#*<option>}" ; [ "$o_tmp" = "$opt_stream" ] && o_tmp="${opt_stream#*<OPTION>}"
                option_list="${option_list}${o_tmp%%</option*}|"
                opt_stream="${opt_stream#*</option>}"
                ;;
            *) 
                opt_stream="" 
                ;;
        esac
    done
    
    add_field_to_state "select" "$f_name" "${option_list%%|*}" "${lbl:-Selection}"
    set_field "OPTIONS" "$TOTAL_FIELDS" "${option_list%|}"
    set_field "OPT_INDEX" "$TOTAL_FIELDS" "1"
}


parse_html() {
    local stream="$1" current_label=""
    TOTAL_FIELDS=0

    while [ -n "$stream" ]; do
        case "$stream" in
            \<*)
                local tag_body="${stream%%>*}" ; tag_body="${tag_body#<}"
                
                case "$tag_body" in
                    input*|INPUT*)
                        parse_input_tag "$tag_body" "$current_label"
                        current_label=""
                        ;;
                    textarea*|TEXTAREA*)
                        parse_textarea_tag "$tag_body" "$stream" "$current_label"
                        current_label=""
                        # Advance main loop stream past closing block tag
                        stream="${stream#*</textarea>}" ; [ "$stream" = "$1" ] && stream="${stream#*</TEXTAREA>}"
                        continue
                        ;;
                    select*|SELECT*)
                        parse_select_tag "$tag_body" "$stream" "$current_label"
                        current_label=""
                        ;;
                    a*|A*)
                        local tmp_href="${tag_body#*href=\'}" ; [ "$tmp_href" = "$tag_body" ] && tmp_href="${tag_body#*href=\"}"
                        add_field_to_state "link" "_link" "${tmp_href%%['\"]*}" ""
                        ;;
                esac
                stream="${stream#*>}"
                ;;
            *)
                local inner_text="${stream%%<*}"
                if [ -n "$(echo "$inner_text" | tr -d ' \t\n\r')" ]; then
                    current_label="$inner_text"
                    if [ "$TOTAL_FIELDS" -gt 0 ]; then
                        [ "$(get_field "TYPE" "$TOTAL_FIELDS")" = "link" ] && set_field "LABEL" "$TOTAL_FIELDS" "$inner_text"
                    fi
                fi
                stream="<${stream#*<}" ; [ "$stream" = "<$stream" ] && stream=""
                ;;
        esac
    done
    add_field_to_state "submit" "_submit" "Save Form Settings" "[ACTION]"
}




# ==============================================================================
# 3. INTERACTIVE UI MODULES
# ==============================================================================
get_selection() {
    local cur_idx="$1" ; shift ; local total_opts=$#
    printf "${ESC}[s" # Save cursor mapping position
    
    stty raw -echo
    while true; do
        eval "local active_val=\"\$$cur_idx\""
        printf "${ESC}[u${ESC}[K< \033[7m%s\033[0m >" "$active_val"

        IFS= read -r -n 1 char
        if [ "$char" = "$ESC" ]; then
            IFS= read -r -n 2 -t 0.1 next_chars
            case "$next_chars" in
                "[D") cur_idx=$((cur_idx - 1)) ; [ "$cur_idx" -lt 1 ] && cur_idx=$total_opts ;; # Left
                "[C") cur_idx=$((cur_idx + 1)) ; [ "$cur_idx" -gt "$total_opts" ] && cur_idx=1 ;;  # Right
            esac
        else
            case "$char" in "") break ;; esac
        fi
    done
    stty cooked echo
    echo "$active_val"
}

handle_file_prompt() {
    stty "$old_tty_settings"
    printf "\r\n\033[KEnter absolute path to local file: "
    read -r user_path
    
    if [ -f "$user_path" ]; then
        set_field "VALUE" "$CUR_FIELD" "$user_path"
    else
        printf "\033[31mError: File not found!\033[0m\r\n" ; sleep 1
    fi
    stty raw -echo
}

render_screen() {
    printf "\033[H\033[J=== SLITAZ TAZPANEL ASH TEXT BROWSER ===\r\n"
    printf "Nav: [UP/DOWN Arrows] | Select: [Enter] | Modify Inputs Directly\r\n"
    printf "======================================================================\r\n"
    
    [ "$CUR_FIELD" -eq 0 ] && printf " \033[7m" || printf "  "
    printf "URL Bar : [ %-55s ]\033[0m\r\n" "$URL_BAR_VALUE"
    printf "======================================================================\r\n\n"
    
    local i=1 ; while [ "$i" -le "$TOTAL_FIELDS" ]; do
        local lbl=$(get_field "LABEL" "$i") ; local val=$(get_field "VALUE" "$i") ; local type=$(get_field "TYPE" "$i")
        [ "$i" -eq "$CUR_FIELD" ] && printf " \033[7m" || printf "  "
        
        case "$type" in
            "password") printf " %-15s : [ %-25s ]" "$lbl" "${val//?/*}" ;;
            "text")     printf " %-15s : [ %-25s ]" "$lbl" "$val" ;;
            "select")   printf " %-15s : < %-25s >" "$lbl" "$val" ;;
            "file")     printf " %-15s : %s" "$lbl" "${val:-[ Clear / Select File... ]}" ;;
            "link")     printf "   <u>[ Link: %-45s ]</u>" "$lbl" ;;
            "submit")   printf "         ==== ( %s ) ====" "$val" ;;
        esac
        printf "\033[0m\r\n" ; i=$((i + 1))
    done
    printf "\n"
}

# ==============================================================================
# 4. FORM SUBMISSION GENERATION LAYER
# ==============================================================================
compile_post_payload() {
    local i=1 
    local payload="" 
    local file_found=""
    
    # Step 1: Pre-scan components to check if a valid File Upload path is active
    while [ "$i" -lt "$TOTAL_FIELDS" ]; do
        if [ "$(get_field "TYPE" "$i")" = "file" ] && [ -n "$(get_field "VALUE" "$i")" ]; then
            # Return raw local file descriptor path instantly to bypass standard post text mapping
            echo "$(get_field "VALUE" "$i")" 
            return 0
        fi
        i=$((i + 1))
    done
    
    # Step 2: Fallback processing pipeline for application URL-encoded string data
    i=1 
    while [ "$i" -lt "$TOTAL_FIELDS" ]; do
        local type=$(get_field "TYPE" "$i")
        
        if [ "$type" = "text" ] || [ "$type" = "password" ] || [ "$type" = "select" ]; then
            local name=$(get_field "NAME" "$i") 
            local val=$(get_field "VALUE" "$i")
            
            # Pure shell URL encoding substitution mappings
            name="${name// /+}" 
            val="${val// /+}"
            
            if [ -z "$payload" ]; then 
                payload="${name}=${val}" 
            else 
                payload="${payload}&${name}=${val}" 
            fi
        fi
        i=$((i + 1))
    done
    
    echo "$payload"
}

# ==============================================================================
# 5. MODULAR KEYBOARD EVENT HANDLERS
# ==============================================================================

navigate_fields() {
    local direction="$1" # "UP" or "DOWN"
    if [ "$direction" = "UP" ]; then
        CUR_FIELD=$((CUR_FIELD - 1))
        [ "$CUR_FIELD" -lt 0 ] && CUR_FIELD=$TOTAL_FIELDS
    else
        CUR_FIELD=$((CUR_FIELD + 1))
        [ "$CUR_FIELD" -gt "$TOTAL_FIELDS" ] && CUR_FIELD=0
    fi
}

handle_enter_action() {
    # If Enter is pressed on the global URL address bar (Index 0)
    if [ "$CUR_FIELD" -eq 0 ]; then
        CURRENT_URL="$URL_BAR_VALUE"
        ACTION_MODE="GET"
        return 0 # Break outer loop code
    fi
    
    local type=$(get_field "TYPE" "$CUR_FIELD")
    case "$type" in
        "link")
            CURRENT_URL=$(resolve_url "$(get_field "VALUE" "$CUR_FIELD")")
            URL_BAR_VALUE="$CURRENT_URL"
            ACTION_MODE="GET"
            return 0 # Break outer loop code
            ;;
        "file")
            handle_file_prompt
            ;;
        "submit")
            ACTION_MODE="POST"
            return 0 # Break outer loop code
            ;;
        "select")
            # Explode the drop-down choices array using your IFS trick
            old_ifs="$IFS" ; IFS="|" ; set -- $(get_field "OPTIONS" "$CUR_FIELD") ; IFS="$old_ifs"
            
            local new_selection=$(get_selection "$(get_field "OPT_INDEX" "$CUR_FIELD")" "$@")
            set_field "VALUE" "$CUR_FIELD" "$new_selection"
            
            # Map selected text back to its corresponding choice number/index
            local idx=1
            for opt in "$@"; do
                [ "$opt" = "$new_selection" ] && set_field "OPT_INDEX" "$CUR_FIELD" "$idx" && break
                idx=$((idx + 1))
            done
            render_screen
            ;;
        *)
            navigate_fields "DOWN"
            ;;
    case
    return 1 # Keep running inner loop code
}

handle_backspace() {
    if [ "$CUR_FIELD" -eq 0 ]; then
        if [ -n "$URL_BAR_VALUE" ]; then
            URL_BAR_VALUE="${URL_BAR_VALUE%?}"
            printf "\b \b"
        fi
    else
        local type=$(get_field "TYPE" "$CUR_FIELD")
        if [ "$type" = "text" ] || [ "$type" = "password" ]; then
            local val=$(get_field "VALUE" "$CUR_FIELD")
            if [ -n "$val" ]; then
                val="${val%?}"
                set_field "VALUE" "$CUR_FIELD" "$val"
                printf "\b \b"
            fi
        fi
    fi
}

append_input_char() {
    local input_char="$1"
    
    # Ensure it's a printable ASCII character
    if [ -n "$(echo "$input_char" | tr -cd '[:print:]')" ]; then
        if [ "$CUR_FIELD" -eq 0 ]; then
            URL_BAR_VALUE="${URL_BAR_VALUE}${input_char}"
        else
            local type=$(get_field "TYPE" "$CUR_FIELD")
            if [ "$type" = "text" ] || [ "$type" = "password" ]; then
                local val=$(get_field "VALUE" "$CUR_FIELD")
                
                # Auto-clear password placeholders on first typed character
                if [ "$type" = "password" ]; then
                    case "$val" in \**|*encrypted*|*hidden*) val="" ;; esac
                fi
                
                set_field "VALUE" "$CUR_FIELD" "${val}${input_char}"
            fi
        fi
    fi
}


# ==============================================================================
# 6. MAIN BROWSER RUNTIME EXECUTIVE ENGINE
# ==============================================================================

# Bootstrap original mock page canvas assets
parse_html "$html"

# Save device terminal state definitions and drop down to unbuffered raw execution
old_tty_settings=$(stty -g)
stty raw -echo

while true; do
    render_screen
    IFS= read -r -n 1 char

    # Intercept ANSI escape values (Arrow Key sequences start with ESC)
    if [ "$char" = "$ESC" ]; then
        IFS= read -r -n 2 -t 0.1 next_chars
        case "$next_chars" in
            "[A") navigate_fields "UP" ;;
            "[B") navigate_fields "DOWN" ;;
        esac
    else
        case "$char" in
            "") # Enter key triggered
                handle_enter_action && break # If function returns 0, drop loop out to dispatch
                ;;
            $'\x7f'|$'\x08') # Backspace variations
                handle_backspace
                ;;
            *) # Standard alphanumeric keys typed
                append_input_char "$char"
                ;;
        esac
    fi
done

# Clear operational layouts out of terminal and restore environment properties
stty "$old_tty_settings"
clear

# ==============================================================================
# 7. NETWORK DISPATCH TRANSMISSION EXECUTION
# ==============================================================================
FINAL_PAYLOAD=$(compile_post_payload)

if [ "$ACTION_MODE" = "POST" ]; then
    echo "Executing CGI POST Update Lifecycle to: $CURRENT_URL"
    echo "Transmission Mode / Payload: $FINAL_PAYLOAD"
    # Live implementation capture:
    # html=$(request_page "POST" "$CURRENT_URL" "$FINAL_PAYLOAD")
else
    echo "Following Hyperlink Anchor/Address Route Path (GET): $CURRENT_URL"
    # Live implementation capture:
    # html=$(request_page "GET" "$CURRENT_URL")
fi




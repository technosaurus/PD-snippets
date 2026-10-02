#!/bin/sh
# A pure BusyBox AWK + Wget Interactive Terminal User Agent

# Ensure terminal drops into unbuffered raw input loop, caching existing flags
OLD_TTY=$(stty -g)
stty raw -echo

# Clean fallback trap to ensure terminal is restored if process dies unexpectedly
trap 'stty "$OLD_TTY"; clear; exit' INT TERM EXIT

# Bootstrapping link entrypoint
INITIAL_URL="https://example.com"

awk -v start_url="$INITIAL_URL" '
function wrap_and_buffer(text, max_cols,    words, num_words, line, i) {
    # Splits strings on space boundaries and writes them to the virtual screen array
    gsub(/[ \t\r\n]+/, " ", text)
    num_words = split(text, words, " ")
    line = "  "
    for (i = 1; i <= num_words; i++) {
        if (length(line) + length(words[i]) + 1 > max_cols - 4) {
            buffer_count++
            screen_buffer[buffer_count] = line
            line = "  " words[i]
        } else {
            line = line (line == "  " ? "" : " ") words[i]
        }
    }
    if (line != "  ") {
        buffer_count++
        screen_buffer[buffer_count] = line
    }
}

function fetch_and_layout(url, post_data,    cmd, tag, text, m_idx, sub_t, q, f_name) {
    # 1. Fire BusyBox wget network stream pipeline
    if (post_data != "") {
        cmd = "wget -q -O - --post-data=\"" post_data "\" \"" url "\""
    } else {
        cmd = "wget -q -O - \"" url "\""
    }

    # Reset structural engine tracking matrix profiles
    link_count = 0; input_count = 0; buffer_count = 0
    form_action = ""; in_script = 0; in_style = 0; in_form = 0

    # 2. Tokenize stream natively using single character separations
    while ((cmd | getline) > 0) {
        tag = $1; text = $2

        # Filter functional context blocks
        if (tag ~ /^[sS][cC][rR][iI][pP][tT]/) { in_script = 1; next }
        if (tag ~ /^\/[sS][cC][rR][iI][pP][tT]/) { in_script = 0; next }
        if (tag ~ /^[sS][tT][yY][lL][eE]/) { in_style = 1; next }
        if (tag ~ /^\/[sS][tT][yY][lL][eE]/) { in_style = 0; next }
        if (in_script || in_style) next

        # Process Anchor Hyperlinks
        if (tag ~ /^[aA][ \t\n]/) {
            m_idx = index(tag, "href=")
            if (m_idx > 0) {
                sub_t = substr(tag, m_idx + 5)
                q = substr(sub_t, 1, 1)
                if (q == "\"" || q == "\047") {
                    sub_t = substr(sub_t, 2)
                    link_count++
                    links[link_count] = substr(sub_t, 1, index(sub_t, q) - 1)
                    
                    # Embed marker into the text stream so the wrap builder maps its selection coordinate
                    if (text ~ /[^ \t\n\r]/) {
                        text = text " [" link_count "]"
                    }
                }
            }
        }

        # Process Form Actions
        if (tag ~ /^[fF][oO][rR][mM]/) {
            in_form = 1
            m_idx = index(tag, "action=")
            if (m_idx > 0) {
                sub_t = substr(tag, m_idx + 7)
                q = substr(sub_t, 1, 1)
                form_action = substr(sub_t, 2, index(substr(sub_t, 2), q) - 1)
            }
            buffer_count++; screen_buffer[buffer_count] = "\033[1;35m[--- FORM START ---]\033[0m"
        }
        if (tag ~ /^\/[fF][oO][rR][mM]/) {
            in_form = 0
            buffer_count++; screen_buffer[buffer_count] = "\033[1;35m[--- FORM END (Type \"f\" to submit) ---]\033[0m"
        }

        # Process Form Fields
        if (in_form && tag ~ /^[iI][nN][pP][uU][tT]/) {
            m_idx = index(tag, "name=")
            if (m_idx > 0) {
                sub_t = substr(tag, m_idx + 5)
                q = substr(sub_t, 1, 1)
                f_name = substr(sub_t, 2, index(substr(sub_t, 2), q) - 1)
                
                input_count++
                inputs[input_count] = f_name
                input_values[input_count] = ""
                
                buffer_count++
                screen_buffer[buffer_count] = "  \033[36m" f_name "\033[0m: [________________] (__INPUT__" input_count "__)"
            }
        }

        # Format Block Layouts vs Line wraps
        if (tag ~ /^[pP]/ || tag ~ /^\/[pP]/ || tag ~ /^[bB][rR]/ || tag ~ /^[hH][1-6]/) {
            if (text ~ /[^ \t\n\r]/) {
                wrap_and_buffer(text, cols)
                buffer_count++; screen_buffer[buffer_count] = "" # Extra spacing line break
            }
        } else if (text ~ /[^ \t\n\r]/) {
            wrap_and_buffer(text, cols)
        }
    }
    close(cmd)
}

function render_paint(    i, line, display_idx) {
    # Render static TUI chrome headers and screen array contents
    printf "\033[2J\033[H" > "/dev/stderr"
    printf "\033[1;37;44m AWK-UA v1.0 | URL: %-50s \033[0m\r\n", current_url > "/dev/stderr"
    printf "\033[1;30;47m Nav: [Up/Down] | Forms: [f] | Back: [b] | Exit: [Esc]                \033[0m\r\n\r\n" > "/dev/stderr"

    for (i = 1; i <= buffer_count; i++) {
        line = screen_buffer[i]
        
        # Colorize link selections globally during screen painting loops
        if (line ~ /\[[0-9]+\]/) {
            gsub(/\[[0-9]+\]/, "\033[32m&\033[0m", line)
        }
        
        # Dynamically inject live input form changes straight into layout canvas lines
        if (line ~ /__INPUT__[0-9]+__/) {
            match(line, /__INPUT__[0-9]+__/)
            display_idx = substr(line, RSTART + 9, RLENGTH - 11)
            # Replace field formatting masks with active tracking memory strings
            gsub(/\[________________\] (__INPUT__[0-9]+__)/, "[\033[4;33m" sprintf("%-16s", substr(input_values[display_idx], 1, 16)) "\033[0m]", line)
        }
        
        printf "%s\r\n", line > "/dev/stderr"
    }
}

BEGIN {
    # Initialize Core Boundary Parsing Markers
    RS = "<"; FS = ">"
    
    # Establish default terminal screen width limits
    cols = 80
    "tput cols" | getline cols
    close("tput cols")

    current_url = start_url
    history_ptr = 0
    next_post_data = ""

    # Primary runtime loop engine thread execution initialization
    while (1) {
        fetch_and_layout(current_url, next_post_data)
        next_post_data = "" # Flash memory map consumption state
        
        while (1) {
            render_paint()
            
            # Unbuffered non-blocking stream monitoring sequence interceptor
            if ((getline key < "/dev/tty") <= 0) exit

            # Intercept Escape Keys and Cursor Tracking Commands
            if (key == "\033") {
                # Read immediate upcoming characters to determine if arrow or standalone Esc
                if ((getline extra < "/dev/tty") > 0) {
                    if (extra == "[A") {
                        # Up arrow tracking placeholder (for multi-page scroll expansions)
                        continue
                    } else if (extra == "[B") {
                        # Down arrow tracking placeholder
                        continue
                    }
                }
                # If no trailing escape characters, it is a standalone Esc stroke -> Terminate
                exit
            }

            # Interactive form field manipulation sequence trigger
            if (key == "f" && form_action != "" && input_count > 0) {
                for (i = 1; i <= input_count; i++) {
                    render_paint()
                    printf "\r\n\033[KEnter value for \033[36m%s\033[0m: ", inputs[i] > "/dev/stderr"
                    stty_cooked = "stty cooked echo"
                    system(stty_cooked)
                    getline val < "/dev/tty"
                    stty_raw = "stty raw -echo"
                    system(stty_raw)
                    input_values[i] = val
                }
                
                # Assemble POST payload
                payload = ""
                for (i = 1; i <= input_count; i++) {
                    payload = payload (i == 1 ? "" : "&") inputs[i] "=" input_values[i]
                }
                
                # Cache navigational trail positions
                history_ptr++
                history[history_ptr] = current_url
                
                next_post_data = payload
                current_url = form_action
                break # Drop back to outer loop to fetch new page via post payload
            }

            # History Stack Navigation (Backtrack)
            if (key == "b" && history_ptr > 0) {
                current_url = history[history_ptr]
                history_ptr--
                break
            }

            # Hyperlink choice navigation mapping evaluation rules
            if (key ~ /[0-9]/) {
                printf "%s", key > "/dev/stderr" # Echo character layout response
                # Pull following numeric components for double-digit entries
                while (1) {
                    if ((getline next_digit < "/dev/tty") <= 0) break
                    if (next_digit ~ /[0-9]/) {
                        key = key next_digit
                        printf "%s", next_digit > "/dev/stderr"
                    } else {
                        break # Executed via trailing enter or alpha input stroke
                    }
                }
                
                target_idx = key + 0
                if (target_idx > 0 && target_idx <= link_count) {
                    # Push active tracking URL to Backstack history
                    history_ptr++
                    history[history_ptr] = current_url
                    
                    current_url = links[target_idx]
                    break
                }
            }
        }
    }
}

END {
    # CLEANUP BLOCK: Safely reset system settings back to normal
    system("stty " OLD_TTY)
    printf "\033[2J\033[HBrowser engine shutdown cleanly.\n" > "/dev/stderr"
}
'

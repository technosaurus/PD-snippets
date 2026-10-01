# The Global Trap Callback Multiplexer
async_handler() {
    # This fires instantly whenever the parent shell catches SIGUSR1
    local name
    for name in $ASYNC_REGISTRY; do
        local pipe_out
        eval "pipe_out=\${${name}_OUT}"
        
        # Non-blocking check to see which specific coproc sent the data
        if [ -s "$pipe_out" ]; then
            local data callback
            read -r data < "$pipe_out" || [ -n "$data" ]
            eval "callback=\${${name}_CALLBACK}"
            
            # Execute the user's defined callback function with the data
            $callback "$data"
        fi
    done
}

# Bind the handler to the standard USR1 signal
trap async_handler USR1

async_spawn() {
    # Usage: async_spawn NAME CALLBACK_FUNC COMMAND [ARGS...]
    local name="$1"
    local callback="$2"
    shift 2

    # Track registered processes so the multiplexer knows who to check
    ASYNC_REGISTRY="$ASYNC_REGISTRY $name"

    local pipe_in="/tmp/async_${$}_${name}_in"
    local pipe_out="/tmp/async_${$}_${name}_out"
    rm -f "$pipe_in" "$pipe_out"
    mkfifo "$pipe_in" "$pipe_out"

    # Store configurations
    eval "${name}_IN=\"\$pipe_in\""
    eval "${name}_OUT=\"\$pipe_out\""
    eval "${name}_CALLBACK=\"\$callback\""

    # Wrap the worker command so it signals the parent PID ($$) via kill 
    # the exact moment it dumps line output into the FIFO
    (
        PARENT_PID=$$
        "$@" < "$pipe_in" | while read -r line || [ -n "$line" ]; do
            echo "$line"
            kill -USR1 $PARENT_PID 2>/dev/null
        done > "$pipe_out"
    ) &

    eval "${name}_PID=$!"
}

async_close() {
    local name="$1"
    local pid
    eval "pid=\${${name}_PID}"
    
    [ -n "$pid" ] && kill -15 "$pid" 2>/dev/null
    eval "rm -f \"\${${name}_IN}\" \"\${${name}_OUT}\""
    
    local padded=" $ASYNC_REGISTRY "
    local before="${padded% $name *}"
    local after="${padded#* $name }"
    ASYNC_REGISTRY="${before# } ${after% }"
    if [ "$ASYNC_REGISTRY" = " " ] || [ "$ASYNC_REGISTRY" = "" ]; then
        ASYNC_REGISTRY=""
    fi
}

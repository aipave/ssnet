#!/bin/bash

# start pingpong_server
start_server() {
  taskset -c 1 ./../../build/release/bin/pingpong_server 0.0.0.0 22222 "$1" "$2" &
  srvpid=$!
}

# stop pingpong_server gracefully
stop_server() {
  kill "$srvpid" && wait "$srvpid" 2>/dev/null
}

# config parameters
timeout=${timeout:-100}
#bufsize=${bufsize:-8192}
bufsize=${bufsize:-16384}
nothreads=1

# sessions  array for testing
sessions=(1 10 100 1000)

# Main loop for running ping-pong tests
for nosessions in "${sessions[@]}"; do
  sleep 5
  echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
  start_server "$nothreads" "$bufsize"
  sleep 1
  taskset -c 2 ./../../build/release/bin/pingpong_client 127.0.0.1 22222 "$nothreads" "$bufsize" "$nosessions" "$timeout"
  echo "Exit code: $?"
  stop_server
done

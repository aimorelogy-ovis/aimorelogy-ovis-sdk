#!/bin/sh
set -eu

manager=$(readlink -f "${1:-ovis-manager/build/ovis-managerd}")
test -x "$manager"
directory=$(mktemp -d /tmp/ovis-log-test-XXXXXX)
trap 'rm -rf "$directory"' EXIT HUP INT TERM

"$manager" --run-logged "$directory/service.log" -- sh -c '
    echo "$$" > "$1"
    dd if=/dev/zero bs=4096 count=180 2>/dev/null
    printf "finished\n"
    exit 7
' logged-writer "$directory/pid" &
pid=$!
status=0
wait "$pid" || status=$?
test "$status" -eq 7
test "$(cat "$directory/pid")" = "$pid"

count=0
while ! tail -c 9 "$directory/service.log" 2>/dev/null | grep -q finished; do
    test "$count" -lt 100
    sleep 0.02
    count=$((count + 1))
done
test "$(wc -c < "$directory/service.log")" -le 262144
test "$(wc -c < "$directory/service.log.1")" -le 262144

# An unavailable log destination must not terminate or block its writer.
"$manager" --run-logged "$directory/missing/service.log" -- sh -c '
    dd if=/dev/zero bs=4096 count=180 2>/dev/null
'
echo "startup log tests passed"

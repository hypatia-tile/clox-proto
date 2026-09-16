#!/bin/bash
BINARY="${1:-./bin/ex01}"
shift 2>/dev/null
DIRS=("$@")
if [ "${#DIRS[@]}" -eq 0 ]; then
  DIRS=(examples)
fi

PASS=0
FAIL=0
TMPFILE=$(mktemp)
TMPERR=$(mktemp)
trap 'rm -f "$TMPFILE" "$TMPERR"' EXIT

for dir in "${DIRS[@]}"; do
  for file in "$dir"/*.lox; do
    [ -e "$file" ] || continue
    expected=$(grep '// expect: ' "$file" | sed 's|.*// expect: ||')
    if [ -z "$expected" ]; then
      echo "FAIL: $file (no // expect: lines)"
      FAIL=$((FAIL + 1))
      continue
    fi

    "$BINARY" "$file" > "$TMPFILE" 2> "$TMPERR"
    ec=$?

    if [ "$ec" -ne 0 ]; then
      echo "FAIL: $file (exit $ec)"
      sed 's/^/  /' "$TMPERR"
      FAIL=$((FAIL + 1))
      continue
    fi

    # Strip debug trace lines: chunk headers (== ...), instruction offsets (4 digits),
    # stack trace lines (leading space), and blank lines.
    actual=$(grep -Ev '^(==|[0-9]{4}| |$)' "$TMPFILE")

    if [ "$expected" = "$actual" ]; then
      PASS=$((PASS + 1))
    else
      echo "FAIL: $file"
      printf '  expected: [%s]\n' "$expected"
      printf '  actual:   [%s]\n' "$actual"
      FAIL=$((FAIL + 1))
    fi
  done
done

echo "$PASS / $((PASS + FAIL)) passed"
[ "$FAIL" -eq 0 ]

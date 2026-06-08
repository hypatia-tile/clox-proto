#!/bin/bash
BINARY="${1:-./bin/ex01}"
PASS=0
FAIL=0
TMPFILE=$(mktemp)
trap 'rm -f "$TMPFILE"' EXIT

for file in examples/*.lox; do
  expected=$(grep '// expect: ' "$file" | sed 's|.*// expect: ||')

  "$BINARY" "$file" > "$TMPFILE" 2>/dev/null
  ec=$?

  if [ "$ec" -ne 0 ]; then
    echo "FAIL: $file (exit $ec)"
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

echo "$PASS / $((PASS + FAIL)) passed"
[ "$FAIL" -eq 0 ]

# `clox` prototyping

This is a prototyping of the `clox` implementation of the Lox language from Bob Nystrom's book, *Crafting Interpreters*.

## Layout

- `ex01/` — the clox interpreter (bytecode VM). All `make` targets below are run from this directory.
  - `src/`, `include/` — interpreter sources
  - `test/` — unit tests for the compiler's bytecode output
  - `examples/` — Lox programs with `// expect:` annotations, used as end-to-end tests
  - `scripts/` — helper scripts (example runner)

## Requirements

- `clang` (C11)
- `make`

## Building and running

Run these from `ex01/`:

```sh
make            # build the release binary (bin/ex01)
make run        # build and run on example/test.txt
make run inputfile=path/to/file.lox   # run on a specific file
make repl       # build and start the REPL
make clean      # remove all build artifacts
```

The release build compiles with `-Wall -Wextra -Werror -std=c11`.

## Testing

```sh
make test       # unit tests: compile Lox snippets and compare emitted bytecode
make tests      # end-to-end tests: run every examples/*.lox and check // expect: output
make gc-tests   # the same examples plus examples/gc/, against a build that
                # collects on every allocation (DEBUG_STRESS_GC)
```

- `make test` builds `test/bin/test_ex01` from `test/src/` linked against the
  interpreter objects (everything except `main.o`). Each case is a table entry
  of source plus expected bytecode/constants; failures print the expected and
  actual disassembly side by side.
- `make tests` runs `scripts/test_examples.sh` against the release binary.
  Each `examples/*.lox` file declares its expected stdout with `// expect:`
  comment lines. The script takes the binary followed by one or more
  directories (`test_examples.sh <binary> [dir...]`, default `examples`).
- `make gc-tests` builds with `GC_FLAGS` (`-DDEBUG_STRESS_GC`) into `gc/` and
  runs both `examples/` and `examples/gc/`. `examples/gc/` holds programs whose
  allocation patterns are the ones a collector gets wrong: string
  concatenation, a constant table that has to grow, interning, and a closure
  outliving the scope it captured. GC logging is deliberately *not* on here,
  because the log lines would be compared against `// expect:` output.

## Debugging

```sh
make debug      # build debug/bin/ex01 and launch it under lldb
make gc-run     # run one file with stress collection and GC logging on
make gc-run inputfile=examples/gc/concat01.lox
make gc-log     # the same run, captured to gclog/gc.log
make gc-log inputfile=examples/while01.lox logfile=/tmp/w.log
make gc-trace   # GC log plus disassembly and execution tracing
make gc-trace-log inputfile=examples/gc/concat01.lox logfile=/tmp/c.log
```

The debug build (`DEBUG_FLAGS` in the Makefile) compiles with `-g -O0` and
defines `DEBUG_PRINT_CODE` (disassemble each chunk after compiling) and
`DEBUG_TRACE_EXECUTION` (trace the stack and each instruction while running).
These macros are **not** defined in the release build, so `make run` and the
test targets produce clean output; use the debug build when you want to see
the emitted bytecode or execution traces.

`make gc-run` builds a third configuration into `gclog/` with both
`GC_FLAGS` (`-DDEBUG_STRESS_GC`) and `GCLOG_FLAGS` (`-DDEBUG_LOG_GC`), and no
instruction tracing — the output is the collector's own log and nothing else.

`make gc-log` runs the same binary but captures the log to a file
(`gclog/gc.log` by default, `logfile=` to change it) and prints the line count
and exit status. Use it instead of redirecting `gc-run` yourself: a C program's
stdout is block-buffered whenever it is not a terminal, so `make gc-run > f`
and `make gc-run | tail` both come back **empty** if the program crashes — the
buffered lines are lost with the process. `gc-log` runs under `stdbuf -o0`, so
every line printed before a crash survives. That matters while debugging the
collector, where a crashing run is exactly the one worth reading.

A crashing run is also more likely under `gclog/` than under `gc/`: with
`DEBUG_LOG_GC` on, `markObject()` calls `printValue()` on the object it is
marking, so a stale object is *dereferenced* rather than merely written to.
A program can therefore pass `make gc-tests` and still fault under
`make gc-run`.

`make gc-trace` is a fourth configuration, built into `gctrace/`: everything
`gclog/` has, plus `DEBUG_PRINT_CODE` and `DEBUG_TRACE_EXECUTION`, compiled
`-g -O0` and without `-Werror` the way the debug build is. Use it when the
question is *which instruction* provoked a collection — each `-- gc begin`
lands in the middle of the traced instruction stream, with the value stack
printed before every instruction, so it is visible whether an operand was
still on the stack when the collector ran. It is far noisier than `gc-run`
(hundreds of lines for a two-line program), so reach for it only once a
smaller log has narrowed things down. `make gc-trace-log` captures it the same
way `gc-log` does.

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
```

- `make test` builds `test/bin/test_ex01` from `test/src/` linked against the
  interpreter objects (everything except `main.o`). Each case is a table entry
  of source plus expected bytecode/constants; failures print the expected and
  actual disassembly side by side.
- `make tests` runs `scripts/test_examples.sh` against the release binary.
  Each `examples/*.lox` file declares its expected stdout with `// expect:`
  comment lines.

## Debugging

```sh
make debug      # build debug/bin/ex01 and launch it under lldb
```

The debug build (`DEBUG_FLAGS` in the Makefile) compiles with `-g -O0` and
defines `DEBUG_PRINT_CODE` (disassemble each chunk after compiling) and
`DEBUG_TRACE_EXECUTION` (trace the stack and each instruction while running).
These macros are **not** defined in the release build, so `make run` and the
test targets produce clean output; use the debug build when you want to see
the emitted bytecode or execution traces.

---
name: crafting-interpreters-test
description: Write or update compiler bytecode tests for this clox (Crafting Interpreters) implementation. Use when adding tests for a new chapter, fixing tests broken by a chapter's compiler changes, or verifying emitted bytecode.
---

# Writing compiler tests for clox

Tests live in `ex01/test/` and run with `make test` (from `ex01/`). The test
binary links every object in `ex01/obj/` except `main.o`, so it exercises the
real compiler.

## Structure

- `test/src/test_compiler.c` — a table-driven suite. Each case is one
  `CompilerTest` entry: a name, a Lox source string, the expected bytecode
  (via the `BYTES(...)` macro), and the expected constant pool (via
  `CONSTANTS(...)`, omit if empty).
- `test/src/test_main.c` — calls `runCompilerTests()` and exits nonzero on
  any failure.
- On mismatch the suite disassembles both the expected and actual chunks, so
  diffs are readable without a debugger.

## Adding a case

1. Read the current codegen in `src/compiler.c` — do not trust bytecode
   shapes from the book or from memory; chapters change them.
2. Add a table entry in `runCompilerTests()`. Compute expected bytes by hand,
   or write a deliberately wrong expectation, run `make test`, and copy the
   "actual" disassembly after verifying it is correct.
3. Run `make test` from `ex01/`.

## Bytecode facts that trip people up (as of chapter 24)

- `compile(source)` returns an `ObjFunction *` (NULL on error); the bytecode
  is in `function->chunk`. There is no out-parameter Chunk anymore.
- Every function ends with an implicit `OP_NIL, OP_RETURN` (two bytes).
- Local slot 0 is reserved for the enclosing function, so the first user
  local is slot 1.
- Jump operands are two bytes (hi, lo) and are relative: forward jumps count
  bytes from just after the operand; `OP_LOOP` offset = (address after the
  operand) - loop target.
- Number constants are NOT deduplicated — `1` appearing twice in the source
  means two constant-pool entries.
- String constants are interned, so an expected value of
  `OBJ_VAL(copyString("f", 1))` compares equal by pointer via `valuesEqual`.
  This requires `initVM()` to have been called first (the suite does this).
- Do not compare `Chunk.capacity` or line numbers — they are implementation
  noise. Compare `count`, code bytes, and constants only.

## Debug output

`DEBUG_PRINT_CODE` and `DEBUG_TRACE_EXECUTION` are only defined for the debug
build (`DEBUG_FLAGS` in the Makefile), so `make test` output is clean. To see
compiler disassembly or execution traces, use `make debug` or build with those
flags defined.

#include "test_compiler.h"

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#include <stdio.h>

// One table entry per test case: the source to compile and the bytecode
// (plus constant pool) the compiler is expected to produce for the
// top-level <script> function.
//
// Chapter 24 notes baked into the expectations below:
//  - compile() returns an ObjFunction*; its chunk holds the bytecode.
//  - Every function ends with an implicit `OP_NIL OP_RETURN`.
//  - Local slot 0 is reserved for the enclosing function, so user
//    locals start at slot 1.
typedef struct {
  const char *name;
  const char *source;
  const uint8_t *code;
  int codeCount;
  const Value *constants;
  int constantCount;
} CompilerTest;

#define BYTES(...)                                                             \
  .code = (const uint8_t[]){__VA_ARGS__},                                      \
  .codeCount = (int)sizeof((const uint8_t[]){__VA_ARGS__})
#define CONSTANTS(...)                                                         \
  .constants = (const Value[]){__VA_ARGS__},                                   \
  .constantCount = (int)(sizeof((const Value[]){__VA_ARGS__}) / sizeof(Value))

static bool matchesExpected(const CompilerTest *test, Chunk *actual) {
  if (actual->count != test->codeCount) {
    return false;
  }
  for (int i = 0; i < test->codeCount; i++) {
    if (actual->code[i] != test->code[i]) {
      return false;
    }
  }
  if (actual->constants.count != test->constantCount) {
    return false;
  }
  for (int i = 0; i < test->constantCount; i++) {
    if (!valuesEqual(actual->constants.values[i], test->constants[i])) {
      return false;
    }
  }
  return true;
}

static void reportMismatch(const CompilerTest *test, Chunk *actual) {
  Chunk expected;
  initChunk(&expected);
  for (int i = 0; i < test->codeCount; i++) {
    writeChunk(&expected, test->code[i], 1);
  }
  for (int i = 0; i < test->constantCount; i++) {
    addConstant(&expected, test->constants[i]);
  }
  disassembleChunk(&expected, "expected");
  disassembleChunk(actual, "actual");
  freeChunk(&expected);
}

static int runTest(const CompilerTest *test) {
  ObjFunction *function = compile(test->source);
  if (function == NULL) {
    printf("FAIL %-28s %s (did not compile)\n", test->name, test->source);
    return 1;
  }
  if (!matchesExpected(test, &function->chunk)) {
    printf("FAIL %-28s %s\n", test->name, test->source);
    reportMismatch(test, &function->chunk);
    return 1;
  }
  printf("PASS %-28s %s\n", test->name, test->source);
  return 0;
}

int runCompilerTests(void) {
  initVM();

  const CompilerTest tests[] = {
      {
          .name = "expression statement",
          .source = "1 + 2;",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(NUMBER_VAL(1), NUMBER_VAL(2)),
      },
      {
          .name = "print statement",
          .source = "print 1 + 2;",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_PRINT, OP_NIL,
                OP_RETURN),
          CONSTANTS(NUMBER_VAL(1), NUMBER_VAL(2)),
      },
      {
          .name = "local var declaration",
          .source = "{ var a = 1; }",
          BYTES(OP_CONSTANT, 0, OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(NUMBER_VAL(1)),
      },
      {
          .name = "local var get",
          .source = "{ var a = 1; print a; }",
          BYTES(OP_CONSTANT, 0, OP_GET_LOCAL, 1, OP_PRINT, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(NUMBER_VAL(1)),
      },
      {
          .name = "local var set",
          .source = "{ var a = 1; a = 2; }",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_SET_LOCAL, 1, OP_POP,
                OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(NUMBER_VAL(1), NUMBER_VAL(2)),
      },
      {
          .name = "nested scopes",
          .source = "{ var a = 1; { var b = 2; } }",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_POP, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(NUMBER_VAL(1), NUMBER_VAL(2)),
      },
      {
          .name = "if without else",
          .source = "if (true) print 1;",
          BYTES(OP_TRUE,                      // condition
                OP_JUMP_IF_FALSE, 0, 7,       // over then branch
                OP_POP, OP_CONSTANT, 0, OP_PRINT,
                OP_JUMP, 0, 1,                // over the else-side pop
                OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(NUMBER_VAL(1)),
      },
      {
          .name = "if with else",
          .source = "if (true) print 1; else print 2;",
          BYTES(OP_TRUE,                      // condition
                OP_JUMP_IF_FALSE, 0, 7,       // over then branch
                OP_POP, OP_CONSTANT, 0, OP_PRINT,
                OP_JUMP, 0, 4,                // over else branch
                OP_POP, OP_CONSTANT, 1, OP_PRINT, OP_NIL, OP_RETURN),
          CONSTANTS(NUMBER_VAL(1), NUMBER_VAL(2)),
      },
      {
          .name = "and",
          .source = "true and false;",
          BYTES(OP_TRUE,                      // left operand
                OP_JUMP_IF_FALSE, 0, 2,       // short-circuit over right
                OP_POP, OP_FALSE,             // discard left, right operand
                OP_POP, OP_NIL, OP_RETURN),
      },
      {
          .name = "or",
          .source = "false or true;",
          BYTES(OP_FALSE,                     // left operand
                OP_JUMP_IF_FALSE, 0, 3,       // falsy: fall into right
                OP_JUMP, 0, 2,                // truthy: short-circuit
                OP_POP, OP_TRUE,              // discard left, right operand
                OP_POP, OP_NIL, OP_RETURN),
      },
      {
          .name = "while",
          .source = "while (false) print 1;",
          BYTES(OP_FALSE,                     // condition (loop start)
                OP_JUMP_IF_FALSE, 0, 7,       // exit loop
                OP_POP, OP_CONSTANT, 0, OP_PRINT,
                OP_LOOP, 0, 11,               // back to condition
                OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(NUMBER_VAL(1)),
      },
      {
          .name = "for with var initializer",
          .source = "for (var i = 0; i < 1; i = i + 1) print i;",
          BYTES(OP_CONSTANT, 0,               // var i = 0 (slot 1)
                OP_GET_LOCAL, 1,              // condition: i < 1
                OP_CONSTANT, 1, OP_LESS,
                OP_JUMP_IF_FALSE, 0, 21,      // exit loop
                OP_POP,
                OP_JUMP, 0, 11,               // over increment to body
                OP_GET_LOCAL, 1,              // increment: i = i + 1
                OP_CONSTANT, 2, OP_ADD, OP_SET_LOCAL, 1, OP_POP,
                OP_LOOP, 0, 23,               // back to condition
                OP_GET_LOCAL, 1, OP_PRINT,    // body: print i
                OP_LOOP, 0, 17,               // back to increment
                OP_POP,                       // condition
                OP_POP,                       // slot for i leaves scope
                OP_NIL, OP_RETURN),
          CONSTANTS(NUMBER_VAL(0), NUMBER_VAL(1), NUMBER_VAL(1)),
      },
      {
          .name = "call",
          .source = "f(1);",
          BYTES(OP_GET_GLOBAL, 0, OP_CONSTANT, 1, OP_CALL, 1, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(OBJ_VAL(copyString("f", 1)), NUMBER_VAL(1)),
      },
  };

  int total = (int)(sizeof(tests) / sizeof(tests[0]));
  int failed = 0;
  for (int i = 0; i < total; i++) {
    failed += runTest(&tests[i]);
  }

  printf("\ncompiler tests: %d passed, %d failed\n", total - failed, failed);
  freeVM();
  return failed;
}

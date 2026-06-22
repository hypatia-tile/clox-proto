#include "compiler.h"
#include "value.h"
#include <stdio.h>

static int fail(const char *message) {
  fprintf(stderr, "Test failed: %s\n", message);
  return 1;
}

static bool chunksEqual(Chunk *a, Chunk *b) {
  if (a->count != b->count || a->capacity != b->capacity) {
    return false;
  }
  for (int i = 0; i < a->count; i++) {
    if (a->code[i] != b->code[i]) {
      return false;
    }
  }
  if (a->constants.count != b->constants.count ||
      a->constants.capacity != b->constants.capacity) {
    return false;
  }
  for (int i = 0; i < a->constants.count; i++) {
    if (!valuesEqual(a->constants.values[i], b->constants.values[i])) {
      return false;
    }
  }
  return true;
}

int test_expr(const char *source, Chunk *expected) {
  Chunk chunk;
  initChunk(&chunk);
  if (!compile(source, &chunk)) {
    return fail("Compilation failed");
  }
  if (!chunksEqual(&chunk, expected)) {
    return fail("Compiled chunk does not match expected");
  }
  freeChunk(&chunk);
  return 0;
}

int test_exprstmt() {
  Chunk expected;
  initChunk(&expected);
  writeChunk(&expected, OP_CONSTANT, 1);
  int lhs = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, lhs, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int rhs = addConstant(&expected, NUMBER_VAL(2));
  writeChunk(&expected, rhs, 1);
  writeChunk(&expected, OP_ADD, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("1 + 2;", &expected);
  freeChunk(&expected);
  return result;
}

int test_printstmt() {
  Chunk expected;
  initChunk(&expected);
  writeChunk(&expected, OP_CONSTANT, 1);
  int lhs = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, lhs, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int rhs = addConstant(&expected, NUMBER_VAL(2));
  writeChunk(&expected, rhs, 1);
  writeChunk(&expected, OP_ADD, 1);
  writeChunk(&expected, OP_PRINT, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("print 1 + 2;", &expected);
  freeChunk(&expected);
  return result;
}

int test_local_var_decl() {
  Chunk expected;
  initChunk(&expected);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("{ var a = 1; }", &expected);
  freeChunk(&expected);
  return result;
}

int test_local_var_get() {
  Chunk expected;
  initChunk(&expected);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx, 1);
  writeChunk(&expected, OP_GET_LOCAL, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, OP_PRINT, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("{ var a = 1; print a; }", &expected);
  freeChunk(&expected);
  return result;
}

int test_local_var_set() {
  Chunk expected;
  initChunk(&expected);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx1 = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx1, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx2 = addConstant(&expected, NUMBER_VAL(2));
  writeChunk(&expected, idx2, 1);
  writeChunk(&expected, OP_SET_LOCAL, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("{ var a = 1; a = 2; }", &expected);
  freeChunk(&expected);
  return result;
}

int test_nested_scope() {
  Chunk expected;
  initChunk(&expected);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx1 = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx1, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx2 = addConstant(&expected, NUMBER_VAL(2));
  writeChunk(&expected, idx2, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("{ var a = 1; { var b = 2; } }", &expected);
  freeChunk(&expected);
  return result;
}

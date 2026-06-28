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

int test_if_then() {
  Chunk expected;
  initChunk(&expected);
  // condition
  writeChunk(&expected, OP_TRUE, 1);
  // JUMP_IF_FALSE over then-body (offset 7 bytes ahead)
  writeChunk(&expected, OP_JUMP_IF_FALSE, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 7, 1);
  // then branch: pop condition, print 1
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx, 1);
  writeChunk(&expected, OP_PRINT, 1);
  // JUMP over else-pop (offset 1 byte ahead)
  writeChunk(&expected, OP_JUMP, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 1, 1);
  // else branch: pop condition (no else body)
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("if (true) print 1;", &expected);
  freeChunk(&expected);
  return result;
}

int test_if_else() {
  Chunk expected;
  initChunk(&expected);
  // condition
  writeChunk(&expected, OP_TRUE, 1);
  // JUMP_IF_FALSE over then-body (offset 7 bytes ahead)
  writeChunk(&expected, OP_JUMP_IF_FALSE, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 7, 1);
  // then branch: pop condition, print 1
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx1 = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx1, 1);
  writeChunk(&expected, OP_PRINT, 1);
  // JUMP over else-body (offset 4 bytes ahead)
  writeChunk(&expected, OP_JUMP, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 4, 1);
  // else branch: pop condition, print 2
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx2 = addConstant(&expected, NUMBER_VAL(2));
  writeChunk(&expected, idx2, 1);
  writeChunk(&expected, OP_PRINT, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("if (true) print 1; else print 2;", &expected);
  freeChunk(&expected);
  return result;
}

int test_and() {
  Chunk expected;
  initChunk(&expected);
  // left operand
  writeChunk(&expected, OP_TRUE, 1);
  // JUMP_IF_FALSE to end, skipping OP_POP + right operand (offset 2 bytes ahead)
  writeChunk(&expected, OP_JUMP_IF_FALSE, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 2, 1);
  // pop left operand and evaluate right
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_FALSE, 1);
  // expression statement pop + return
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("true and false;", &expected);
  freeChunk(&expected);
  return result;
}

int test_or() {
  Chunk expected;
  initChunk(&expected);
  // left operand
  writeChunk(&expected, OP_FALSE, 1);
  // JUMP_IF_FALSE past the unconditional jump (offset 3 bytes ahead)
  writeChunk(&expected, OP_JUMP_IF_FALSE, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 3, 1);
  // left was truthy: JUMP over OP_POP + right operand (offset 2 bytes ahead)
  writeChunk(&expected, OP_JUMP, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 2, 1);
  // left was falsy: pop left, evaluate right
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_TRUE, 1);
  // expression statement pop + return
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("false or true;", &expected);
  freeChunk(&expected);
  return result;
}

int test_while() {
  Chunk expected;
  initChunk(&expected);
  // loopStart = 0; condition: false
  writeChunk(&expected, OP_FALSE, 1);
  // JUMP_IF_FALSE over pop+body+loop (offset 7)
  writeChunk(&expected, OP_JUMP_IF_FALSE, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 7, 1);
  // truthy path: pop condition, body: print 1
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx, 1);
  writeChunk(&expected, OP_PRINT, 1);
  // loop back to loopStart=0; after emitting OP_LOOP count=9, offset=9-0+2=11
  writeChunk(&expected, OP_LOOP, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 11, 1);
  // falsy path: pop condition
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("while (false) print 1;", &expected);
  freeChunk(&expected);
  return result;
}

int test_for_with_var() {
  Chunk expected;
  initChunk(&expected);
  // initializer: var i = 0
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx0 = addConstant(&expected, NUMBER_VAL(0));
  writeChunk(&expected, idx0, 1);
  // loopStart=2; condition: i < 1
  writeChunk(&expected, OP_GET_LOCAL, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx1 = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx1, 1);
  writeChunk(&expected, OP_LESS, 1);
  // JUMP_IF_FALSE to exit (exitJump=8); patched to offset 21
  writeChunk(&expected, OP_JUMP_IF_FALSE, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 21, 1);
  // truthy: pop condition
  writeChunk(&expected, OP_POP, 1);
  // JUMP over increment to body (bodyJump=12); patched to offset 11
  writeChunk(&expected, OP_JUMP, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 11, 1);
  // incrementStart=14; increment: i = i + 1
  writeChunk(&expected, OP_GET_LOCAL, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int idx2 = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, idx2, 1);
  writeChunk(&expected, OP_ADD, 1);
  writeChunk(&expected, OP_SET_LOCAL, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, OP_POP, 1);
  // loop to condition at loopStart=2; after emitting OP_LOOP count=23, offset=23-2+2=23
  writeChunk(&expected, OP_LOOP, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 23, 1);
  // body (starts at 25): print i
  writeChunk(&expected, OP_GET_LOCAL, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, OP_PRINT, 1);
  // loop to increment at incrementStart=14; after emitting OP_LOOP count=29, offset=29-14+2=17
  writeChunk(&expected, OP_LOOP, 1);
  writeChunk(&expected, 0, 1);
  writeChunk(&expected, 17, 1);
  // falsy: pop condition; endScope: pop local i
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_POP, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("for (var i = 0; i < 1; i = i + 1) print i;", &expected);
  freeChunk(&expected);
  return result;
}

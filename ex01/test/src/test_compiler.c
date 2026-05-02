#include "compiler.h"
#include "value.h"

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

int test_binary() {
  Chunk expected;
  initChunk(&expected);
  writeChunk(&expected, OP_CONSTANT, 1);
  int rhs = addConstant(&expected, NUMBER_VAL(1));
  writeChunk(&expected, rhs, 1);
  writeChunk(&expected, OP_CONSTANT, 1);
  int lhs = addConstant(&expected, NUMBER_VAL(2));
  writeChunk(&expected, lhs, 1);
  writeChunk(&expected, OP_ADD, 1);
  writeChunk(&expected, OP_RETURN, 1);

  int result = test_expr("1 + 2", &expected);
  freeChunk(&expected);
  return result;
};

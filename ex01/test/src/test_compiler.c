#include "test_compiler.h"

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#include <stdio.h>
#include <string.h>

// An entry in an expected constant pool. Plain values (numbers, strings)
// compare with valuesEqual; function constants have no stable identity to
// compare against, so they are described by name/arity/upvalueCount instead.
typedef struct {
  bool isFunction;
  Value value;         // when isFunction is false
  const char *name;    // when isFunction is true
  int arity;
  int upvalueCount;
} ExpectedConstant;

// One table entry per test case: the source to compile and the bytecode
// (plus constant pool) the compiler is expected to produce. By default the
// expectation applies to the top-level <script>; set .inFunction to check a
// nested function's chunk instead.
//
// Notes baked into the expectations below:
//  - compile() returns an ObjFunction*; its chunk holds the bytecode.
//  - Every function ends with an implicit `OP_NIL OP_RETURN`.
//  - Local slot 0 is reserved for the enclosing function, so user
//    locals start at slot 1.
//  - Chapter 25: a function declaration emits OP_CLOSURE, followed by two
//    bytes per upvalue (isLocal flag, then index).
//  - Chapter 27: a class declaration emits OP_CLASS with the name's constant
//    index, then binds the name the same way `var` does.
typedef struct {
  const char *name;
  const char *source;
  const char *inFunction; // NULL => the top-level <script>
  const uint8_t *code;
  int codeCount;
  const ExpectedConstant *constants;
  int constantCount;
} CompilerTest;

#define BYTES(...)                                                             \
  .code = (const uint8_t[]){__VA_ARGS__},                                      \
  .codeCount = (int)sizeof((const uint8_t[]){__VA_ARGS__})
#define CONSTANTS(...)                                                         \
  .constants = (const ExpectedConstant[]){__VA_ARGS__},                        \
  .constantCount = (int)(sizeof((const ExpectedConstant[]){__VA_ARGS__}) /     \
                         sizeof(ExpectedConstant))
#define VALUE(v) {.isFunction = false, .value = (v)}
#define FUNCTION(n, a, u)                                                      \
  {.isFunction = true, .name = (n), .arity = (a), .upvalueCount = (u)}

// Depth-first search of the constant pools for the function named `name`.
static ObjFunction *findFunction(ObjFunction *function, const char *name) {
  for (int i = 0; i < function->chunk.constants.count; i++) {
    Value constant = function->chunk.constants.values[i];
    if (!IS_FUNCTION(constant))
      continue;

    ObjFunction *nested = AS_FUNCTION(constant);
    if (nested->name != NULL && strcmp(nested->name->chars, name) == 0) {
      return nested;
    }

    ObjFunction *found = findFunction(nested, name);
    if (found != NULL)
      return found;
  }
  return NULL;
}

static bool constantMatches(const ExpectedConstant *expected, Value actual) {
  if (!expected->isFunction) {
    return valuesEqual(actual, expected->value);
  }
  if (!IS_FUNCTION(actual))
    return false;

  ObjFunction *function = AS_FUNCTION(actual);
  const char *actualName = function->name == NULL ? "" : function->name->chars;
  return strcmp(actualName, expected->name) == 0 &&
         function->arity == expected->arity &&
         function->upvalueCount == expected->upvalueCount;
}

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
    if (!constantMatches(&test->constants[i], actual->constants.values[i])) {
      return false;
    }
  }
  return true;
}

// The disassembly alone cannot show arity or upvalue counts, so spell out
// constant-pool differences before printing it.
static void reportConstantDiff(const CompilerTest *test, Chunk *actual) {
  if (actual->constants.count != test->constantCount) {
    printf("  constants: expected %d, got %d\n", test->constantCount,
           actual->constants.count);
  }

  int shared = actual->constants.count < test->constantCount
                   ? actual->constants.count
                   : test->constantCount;
  for (int i = 0; i < shared; i++) {
    const ExpectedConstant *expected = &test->constants[i];
    Value got = actual->constants.values[i];
    if (constantMatches(expected, got))
      continue;

    printf("  constant %d: expected ", i);
    if (expected->isFunction) {
      printf("<fn %s> (arity %d, upvalues %d)", expected->name, expected->arity,
             expected->upvalueCount);
    } else {
      printValue(expected->value);
    }
    printf(", got ");
    printValue(got);
    if (IS_FUNCTION(got)) {
      ObjFunction *function = AS_FUNCTION(got);
      printf(" (arity %d, upvalues %d)", function->arity,
             function->upvalueCount);
    }
    printf("\n");
  }
}

static void reportMismatch(const CompilerTest *test, Chunk *actual) {
  Chunk expected;
  initChunk(&expected);
  for (int i = 0; i < test->codeCount; i++) {
    writeChunk(&expected, test->code[i], 1);
  }
  for (int i = 0; i < test->constantCount; i++) {
    const ExpectedConstant *constant = &test->constants[i];
    if (!constant->isFunction) {
      addConstant(&expected, constant->value);
      continue;
    }
    // Stand-in object so the disassembly of OP_CLOSURE reads naturally.
    ObjFunction *function = newFunction();
    function->arity = constant->arity;
    function->upvalueCount = constant->upvalueCount;
    function->name = copyString(constant->name, (int)strlen(constant->name));
    addConstant(&expected, OBJ_VAL(function));
  }
  reportConstantDiff(test, actual);
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
  if (test->inFunction != NULL) {
    function = findFunction(function, test->inFunction);
    if (function == NULL) {
      printf("FAIL %-28s %s (no function named \"%s\")\n", test->name,
             test->source, test->inFunction);
      return 1;
    }
  }
  if (!matchesExpected(test, &function->chunk)) {
    printf("FAIL %-28s %s\n", test->name, test->source);
    reportMismatch(test, &function->chunk);
    return 1;
  }
  printf("PASS %-28s %s\n", test->name, test->source);
  return 0;
}

// Sources shared by several closure cases, so the expectations below can be
// read side by side.
#define CLOSURE_SRC "fun outer() { var x = 1; fun inner() { print x; } }"
#define FLAT_SRC                                                               \
  "fun outer() { var x = 1; fun middle() { fun inner() { print x; } } }"

int runCompilerTests(void) {
  initVM();

  const CompilerTest tests[] = {
      {
          .name = "expression statement",
          .source = "1 + 2;",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), VALUE(NUMBER_VAL(2))),
      },
      {
          .name = "print statement",
          .source = "print 1 + 2;",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_PRINT, OP_NIL,
                OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), VALUE(NUMBER_VAL(2))),
      },
      {
          .name = "local var declaration",
          .source = "{ var a = 1; }",
          BYTES(OP_CONSTANT, 0, OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1))),
      },
      {
          .name = "local var get",
          .source = "{ var a = 1; print a; }",
          BYTES(OP_CONSTANT, 0, OP_GET_LOCAL, 1, OP_PRINT, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1))),
      },
      {
          .name = "local var set",
          .source = "{ var a = 1; a = 2; }",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_SET_LOCAL, 1, OP_POP,
                OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), VALUE(NUMBER_VAL(2))),
      },
      {
          .name = "nested scopes",
          .source = "{ var a = 1; { var b = 2; } }",
          BYTES(OP_CONSTANT, 0, OP_CONSTANT, 1, OP_POP, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), VALUE(NUMBER_VAL(2))),
      },
      {
          .name = "if without else",
          .source = "if (true) print 1;",
          BYTES(OP_TRUE,                      // condition
                OP_JUMP_IF_FALSE, 0, 7,       // over then branch
                OP_POP, OP_CONSTANT, 0, OP_PRINT,
                OP_JUMP, 0, 1,                // over the else-side pop
                OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1))),
      },
      {
          .name = "if with else",
          .source = "if (true) print 1; else print 2;",
          BYTES(OP_TRUE,                      // condition
                OP_JUMP_IF_FALSE, 0, 7,       // over then branch
                OP_POP, OP_CONSTANT, 0, OP_PRINT,
                OP_JUMP, 0, 4,                // over else branch
                OP_POP, OP_CONSTANT, 1, OP_PRINT, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), VALUE(NUMBER_VAL(2))),
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
          CONSTANTS(VALUE(NUMBER_VAL(1))),
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
          CONSTANTS(VALUE(NUMBER_VAL(0)), VALUE(NUMBER_VAL(1)), VALUE(NUMBER_VAL(1))),
      },
      {
          .name = "call",
          .source = "f(1);",
          BYTES(OP_GET_GLOBAL, 0, OP_CONSTANT, 1, OP_CALL, 1, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(VALUE(OBJ_VAL(copyString("f", 1))), VALUE(NUMBER_VAL(1))),
      },
      // --- Chapter 25: closures ---------------------------------------
      {
          .name = "function declaration",
          .source = "fun f() {}",
          BYTES(OP_CLOSURE, 1,          // no upvalues, so no trailing operands
                OP_DEFINE_GLOBAL, 0, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(OBJ_VAL(copyString("f", 1))), FUNCTION("f", 0, 0)),
      },
      {
          .name = "closure captures local",
          .source = CLOSURE_SRC,
          .inFunction = "outer",
          BYTES(OP_CONSTANT, 0,         // var x = 1 (slot 1)
                OP_CLOSURE, 1, 1, 1,    // <fn inner>, capturing local slot 1
                OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), FUNCTION("inner", 0, 1)),
      },
      {
          .name = "upvalue get",
          .source = CLOSURE_SRC,
          .inFunction = "inner",
          BYTES(OP_GET_UPVALUE, 0, OP_PRINT, OP_NIL, OP_RETURN),
      },
      {
          .name = "upvalue set",
          .source = "fun outer() { var x = 1; fun inner() { x = 2; } }",
          .inFunction = "inner",
          BYTES(OP_CONSTANT, 0, OP_SET_UPVALUE, 0, OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(2))),
      },
      {
          // Two references to x share one upvalue, so OP_CLOSURE still
          // carries a single operand pair.
          .name = "upvalue reused",
          .source = "fun outer() { var x = 1; fun inner() { print x; print x; } }",
          .inFunction = "outer",
          BYTES(OP_CONSTANT, 0, OP_CLOSURE, 1, 1, 1, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), FUNCTION("inner", 0, 1)),
      },
      {
          // middle does not mention x, but has to capture it so inner can
          // reach it.
          .name = "flattened upvalue (middle)",
          .source = FLAT_SRC,
          .inFunction = "outer",
          BYTES(OP_CONSTANT, 0, OP_CLOSURE, 1, 1, 1, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), FUNCTION("middle", 0, 1)),
      },
      {
          // inner captures middle's upvalue 0, not a local: isLocal is 0.
          .name = "flattened upvalue (inner)",
          .source = FLAT_SRC,
          .inFunction = "middle",
          BYTES(OP_CLOSURE, 0, 0, 0, OP_NIL, OP_RETURN),
          CONSTANTS(FUNCTION("inner", 0, 1)),
      },
      {
          // Leaving the block pops the uncaptured local (inner) but closes
          // the captured one (x).
          .name = "captured local closed",
          .source = "{ var x = 1; fun inner() { print x; } }",
          BYTES(OP_CONSTANT, 0, OP_CLOSURE, 1, 1, 1, OP_POP, OP_CLOSE_UPVALUE,
                OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(NUMBER_VAL(1)), FUNCTION("inner", 0, 1)),
      },
      // --- Chapter 27: classes ----------------------------------------
      {
          // A class declaration is a variable declaration whose initialiser
          // is OP_CLASS: the class object is built on the stack first, then
          // bound to the name like any other global.
          .name = "class declaration",
          .source = "class Foo {}",
          BYTES(OP_CLASS, 0, OP_DEFINE_GLOBAL, 0, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(OBJ_VAL(copyString("Foo", 3)))),
      },
      {
          // In a local scope there is no OP_DEFINE_GLOBAL: the class object
          // stays in its stack slot, and the block's end pops it.
          .name = "local class declaration",
          .source = "{ class Foo {} }",
          BYTES(OP_CLASS, 0, OP_POP, OP_NIL, OP_RETURN),
          CONSTANTS(VALUE(OBJ_VAL(copyString("Foo", 3)))),
      },
      {
          // The name resolves as a local, which it only can if the class
          // declaration declared it before emitting OP_CLASS.
          .name = "local class resolves as a local",
          .source = "{ class Foo {} print Foo; }",
          BYTES(OP_CLASS, 0, OP_GET_LOCAL, 1, OP_PRINT, OP_POP, OP_NIL,
                OP_RETURN),
          CONSTANTS(VALUE(OBJ_VAL(copyString("Foo", 3)))),
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

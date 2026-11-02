#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

/**
 * @brief The opcodes the VM executes.
 *
 * Each instruction is one opcode byte, followed by its operand bytes, if any.
 */
typedef enum {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_UPVALUE,
  OP_SET_UPVALUE,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_CALL,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_RETURN,
} OpCode;

/**
 * @brief Holds a dynamic array of bytecode, line numbers, and constants.
 *
 * Represents a raw sequence of bytecode as a dynamic array @ref code. Roughly
 * speaking, a program consists of instruction sequences and corresponding
 * constant pools. A constant pool @ref constants is also a dynamic array. An
 * instruction may have an operand that is an index into @ref constants, so that
 * the virtual machine can find the constant it uses. For the sake of
 * traceability, a chunk also has line information @ref lines: for each byte
 * offset in @ref code, @ref lines holds the source line it came from. Hence
 * @ref code and @ref lines have the same length.
 */
typedef struct {
  int count;     ///< Number of bytes currently used in @ref code and
                 ///< @ref lines.
  int capacity;  ///< Allocated capacity of @ref code and @ref lines.
  uint8_t *code; ///< Dynamic array holding the bytecode: opcodes and their
                 ///< operands.
  int *lines;    ///< Dynamic array mapping each byte offset to its source line
                 ///< number, used for debugging and error reporting.
  ValueArray constants; ///< Constant pool: values that instructions refer to by
                        ///< index (literals, global names, functions).
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);
int addConstant(Chunk *chunk, Value value);

#endif // clox_chunk_h

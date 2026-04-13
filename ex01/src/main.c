#include "common.h"
#include "chunk.h"

int main(void) {
  Chunk chunk;
  initChunk(&chunk);
  writeChunk(&chunk, OP_RETURN);
  for (size_t i = 0; i < sizeof(chunk.code); i++) {
    printf("%hhu", (chunk.code)[i]);
  }
  freeChunk(&chunk);
  return 0;
}

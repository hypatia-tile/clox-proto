#include "test_compiler.h"
int main(void) { 
  int i = 0;
  i += test_exprstmt();
  i += test_printstmt();
  return i;
}

#include "test_compiler.h"
int main(void) { 
  int i = 0;
  i += test_exprstmt();
  i += test_printstmt();
  i += test_local_var_decl();
  i += test_local_var_get();
  i += test_local_var_set();
  i += test_nested_scope();
  return i;
}

#include "emulator.h"
#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Open any necessary files for test suite here */
int
init_opcodes_suite(void)
{
  return 0;
}

/* Close anything that was opened here for test suite */
int
clean_suite(void)
{
  return 0;
}

/* Add tests here */
void
test_opcode_0x01(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0x0001, 0xFF);
  cpu_write_mem(&cpu, 0x0002, 0x12);

  int code_found = execute_instruction(&cpu, 0x01); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT(cpu.c == 0xFF);
  CU_ASSERT(cpu.b == 0x12);
}

void
test_opcode_0x0d(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.c = 0x02;

  // decrease C from 2 to 1
  int code_found = execute_instruction(&cpu, 0x0d); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.c == 0x01);
  CU_ASSERT((cpu.flags & FLAG_P) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);

  // decrease C from 1 to 0
  code_found = execute_instruction(&cpu, 0x0d); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 2);
  CU_ASSERT(cpu.c == 0x00);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == FLAG_Z);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);

  // decrease C from 0 to 255
  code_found = execute_instruction(&cpu, 0x0d); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT(cpu.c == 0xFF);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == FLAG_S);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);
}

void
test_opcode_0x13(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  int code_found = execute_instruction(&cpu, 0x13); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.e == 1);
  CU_ASSERT(cpu.d == 0);

  cpu.e = 0xFF;                                 // NOLINT
  code_found = execute_instruction(&cpu, 0x13); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 2);
  CU_ASSERT(cpu.e == 0);
  CU_ASSERT(cpu.d == 1);
}

void
test_opcode_0x23(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  int code_found = execute_instruction(&cpu, 0x23); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.l == 1);
  CU_ASSERT(cpu.h == 0);

  cpu.l = 0xFF;                                 // NOLINT
  code_found = execute_instruction(&cpu, 0x23); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 2);
  CU_ASSERT(cpu.l == 0);
  CU_ASSERT(cpu.h == 1);
}

int
main(void)
{
  CU_pSuite pSuite = NULL;

  if (CUE_SUCCESS != CU_initialize_registry())
    {
      return CU_get_error();
    }

  pSuite = CU_add_suite("opcodes", init_opcodes_suite, clean_suite);
  if (NULL == pSuite)
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  /*
   * Add new tests to this statement.  Replace the second argument of
   * CU_add_test with an output string for running the test, and the third
   * argument with the test function name.
   */
  if ((NULL
       == CU_add_test(pSuite, "test of test_opcode_0x01()", test_opcode_0x01))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x0d()",
                         test_opcode_0x0d))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x13()",
                         test_opcode_0x13))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x23()",
                         test_opcode_0x23)))
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}

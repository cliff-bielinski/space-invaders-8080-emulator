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

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
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

void
test_opcode_0x32(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0x0001, 0xAA);
  cpu_write_mem(&cpu, 0x0002, 0xBB);

  cpu.a = 0x11;

  int code_found = execute_instruction(&cpu, 0x32); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT(cpu_read_mem(&cpu, 0xBBAA) == cpu.a);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0x56(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0xBBAA, 0x11);

  cpu.h = 0xBB;
  cpu.l = 0xAA;

  int code_found = execute_instruction(&cpu, 0x56); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.d == cpu_read_mem(&cpu, 0xBBAA));

  // clean up
  cpu_write_mem(&cpu, 0xBBAA, 0x00);
}

void
test_opcode_0x77(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.a = 0x11;

  cpu.h = 0xBB;
  cpu.l = 0xAA;

  int code_found = execute_instruction(&cpu, 0x77); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu_read_mem(&cpu, 0xBBAA) == cpu.a);

  // clean up
  cpu_write_mem(&cpu, 0xBBAA, 0x00);
}

void
test_opcode_0x7e(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0xBBAA, 0x11);

  cpu.h = 0xBB;
  cpu.l = 0xAA;

  int code_found = execute_instruction(&cpu, 0x7e); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.a == cpu_read_mem(&cpu, 0xBBAA));

  // clean up
  cpu_write_mem(&cpu, 0xBBAA, 0x00);
}

void
test_opcode_0xc2(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0x0001, 0xAA);
  cpu_write_mem(&cpu, 0x0002, 0xBB);

  // set flag to Zero
  update_zero_flag(&cpu, 0x00);

  int code_found = execute_instruction(&cpu, 0xc2); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT((cpu.flags & FLAG_Z) == FLAG_Z);

  // set flag to Non-Zero
  update_zero_flag(&cpu, 0x01);
  cpu.pc = 0;

  code_found = execute_instruction(&cpu, 0xc2); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0xBBAA);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0xc9(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  // write memory address in stack pointer
  cpu_write_mem(&cpu, 0x0001, 0xAA);
  cpu_write_mem(&cpu, 0x0002, 0xBB);

  cpu.sp = 0x0001;

  int code_found = execute_instruction(&cpu, 0xc9); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0xBBAA);
  CU_ASSERT(cpu.sp == 0x0003);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0xd5(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.d = 0xBB;
  cpu.e = 0xAA;
  cpu.sp = 0xFFFF;

  int code_found = execute_instruction(&cpu, 0xd5); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0x0001);
  CU_ASSERT(cpu.sp == 0xFFFD);
  CU_ASSERT(cpu_read_mem(&cpu, cpu.sp) == cpu.e);
  CU_ASSERT(cpu_read_mem(&cpu, cpu.sp + 1) == cpu.d);

  // clean up
  cpu_write_mem(&cpu, 0xFFFE, 0x00);
  cpu_write_mem(&cpu, 0xFFFD, 0x00);
}

void
test_opcode_0xeb(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.d = 0xAA;
  cpu.e = 0xBB;
  cpu.h = 0xCC;
  cpu.l = 0xDD;

  int code_found = execute_instruction(&cpu, 0xeb); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0x0001);
  CU_ASSERT(cpu.d == 0xCC);
  CU_ASSERT(cpu.e == 0xDD);
  CU_ASSERT(cpu.h == 0xAA);
  CU_ASSERT(cpu.l == 0xBB);
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
                         test_opcode_0x23))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x32()",
                         test_opcode_0x32))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x56()",
                         test_opcode_0x56))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x77()",
                         test_opcode_0x77))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x7e()",
                         test_opcode_0x7e))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc2()",
                         test_opcode_0xc2))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc9()",
                         test_opcode_0xc9))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xd5()",
                         test_opcode_0xd5))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xeb()",
                         test_opcode_0xeb)))
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}

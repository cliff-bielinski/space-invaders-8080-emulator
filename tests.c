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
test_opcode_0x06(void) // NOLINT
{ // MVI B, mem

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu_write_mem(&cpu, 0x0001, 0xCC);

  // execute
  int code_found = execute_instruction(&cpu, 0x06);

  // verify 
  CU_ASSERT(0 == code_found);
  CU_ASSERT(2 == cpu.pc);
  CU_ASSERT(0xCC == cpu.b);

  // cleanup
  cpu_write_mem(&cpu, 0x0001, 0x00);
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
test_opcode_0x0f(void) // NOLINT
{ // RRC 

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.a = 0xCC; 

  // execute
  int code_found = execute_instruction(&cpu, 0x0f); // NOLINT

  // verify 
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x66 == cpu.a); // NOLINT
  CU_ASSERT(FLAG_CY == (cpu.flags & FLAG_CY));
}

void
test_opcode_0x1a(void) // NOLINT
{ // LDAX D 

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu_write_mem(&cpu, 0x0001, 0xCC); // NOLINT
  cpu.d = 0x01; // NOLINT
  cpu.e = 0x00; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x1a); // NOLINT

  // verify 
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0xCC == cpu.d); // NOLINT

  // cleanup
  cpu_write_mem(&cpu, 0x0001, 0x00); // NOLINT
}

void
test_opcode_0x29(void) // NOLINT
{ // DAD H 

  /* no carry test */
  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.h = 0xaa; // NOLINT
  cpu.l = 0x11; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x29); // NOLINT

  // verify 
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x54 == cpu.h); // NOLINT
  CU_ASSERT(0x23 == cpu.l); // NOLINT
  CU_ASSERT(FLAG_CY != (cpu.flags & FLAG_CY)); 

  /* carry test */
  // setup
  cpu.h = 0x00; // NOLINT
  cpu.l = 0x80; // NOLINT
  
  // execute
  code_found = execute_instruction(&cpu, 0x29); // NOLINT

  // verify 
  CU_ASSERT(0 == code_found);
  CU_ASSERT(2 == cpu.pc);
  CU_ASSERT(0x00 == cpu.h); // NOLINT
  CU_ASSERT(0x00 == cpu.l); // NOLINT
  CU_ASSERT(FLAG_CY == (cpu.flags & FLAG_CY));
}

void
test_opcode_0x3a(void) // NOLINT
{ // LDA adr  

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu_write_mem(&cpu, 0x0001, 0xAB); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0xCD); // NOLINT
  cpu_write_mem(&cpu, 0xcdab, 0x42); // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x3a); // NOLINT

  // verify 
  CU_ASSERT(0 == code_found);
  CU_ASSERT(3 == cpu.pc);
  CU_ASSERT(0x42 == cpu.a); // NOLINT

  // cleanup
  cpu_write_mem(&cpu, 0x0001, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xcdab, 0x00); // NOLINT
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
  if ((NULL == CU_add_test(pSuite, "test of test_opcode_0x06()", test_opcode_0x06))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x13()", test_opcode_0x13))
      || (NULL == CU_add_test(pSuite, "test of test_opcode_0x0f()", test_opcode_0x0f))
      || (NULL == CU_add_test(pSuite, "test of test_opcode_0x1a()", test_opcode_0x1a))
      || (NULL == CU_add_test(pSuite, "test of test_opcode_0x29()", test_opcode_0x29))
      || (NULL == CU_add_test(pSuite, "test of test_opcode_0x29()", test_opcode_0x3a)))
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}

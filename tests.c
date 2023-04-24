<<<<<<< HEAD
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "emulator.h"
=======
#include <stdio.h>
#include <CUnit/Basic.h>
/* Need to include header file to import structures and emulator function */
>>>>>>> develop

/* Open any necessary files for test suite here */
int init_opcodes_suite(void)
{
	return 0;
}

/* Close anything that was opened here for test suite */
int clean_suite(void)
{
	return 0;
}

/* Add tests here */
void test_func(void)
{
	;
}

void test_func2(void)
{
	;
}

<<<<<<< HEAD
void test_opcode_0x13(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  execute_instruction(&cpu, 0x13);

  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.e == 1);
  CU_ASSERT(cpu.d == 0);
}

=======
>>>>>>> develop
int main(void)
{
	CU_pSuite pSuite = NULL;

	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	pSuite = CU_add_suite("opcodes", init_opcodes_suite, clean_suite);
		if (NULL == pSuite) {
			CU_cleanup_registry();
			return CU_get_error();
	   	}
	
	/* 
	 * Add new tests to this statement.  Replace the second argument of
	 * CU_add_test with an output string for running the test, and the third
	 * argument with the test function name.
	*/
	if ((NULL == CU_add_test(pSuite, "test of test_func()", test_func)) ||
<<<<<<< HEAD
		(NULL == CU_add_test(pSuite, "test of test_func2()", test_func2)) ||
    (NULL == CU_add_test(pSuite, "test of test_opcode_0x13()", test_func2)))
=======
		(NULL == CU_add_test(pSuite, "test of test_func2()", test_func2)))
>>>>>>> develop
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
<<<<<<< HEAD
}
=======
}
>>>>>>> develop

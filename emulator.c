#include "emulator.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Execute Instruction
void execute_instruction(i8080 *cpu, uint8_t opcode) {
  switch (opcode) {
  case 0x00: // NOLINT
    printf("NOP");
    break;
  case 0x01: // NOLINT
    printf("LXI B");
    break;
  case 0x05: // NOLINT
    printf("DCR B");
    break;
  case 0x06: // NOLINT
    printf("MVI B");
    break;
  case 0x09: // NOLINT
    printf("DAD B");
    break;
  case 0x0d: // NOLINT
    printf("DCR C");
    break;
  case 0x0e: // NOLINT
    printf("MVI C");
    break;
  case 0x0f: // NOLINT
    printf("RRC");
    break;
  case 0x11: // NOLINT
    printf("LXI D");
    break;
  case 0x13: // NOLINT
  {          // INX D
    cpu->e += 1;

    if (cpu->e == 0) {
      cpu->d += 1;
    }

    cpu->pc += 1;
    break;
  }
  case 0x19: // NOLINT
    printf("DAD D");
    break;
  case 0x1a: // NOLINT
    printf("LDAX D");
    break;
  case 0x21: // NOLINT
    printf("LXI H");
    break;
  case 0x23: // NOLINT
    printf("INX H");
    break;
  case 0x26: // NOLINT
    printf("MVI H");
    break;
  case 0x29: // NOLINT
    printf("DAD H");
    break;
  case 0x31: // NOLINT
    printf("LXI SP");
    break;
  case 0x32: // NOLINT
    printf("STA");
    break;
  case 0x36: // NOLINT
    printf("MVI M");
    break;
  case 0x3a: // NOLINT
    printf("LDA ");
    break;
  case 0x3e: // NOLINT
    printf("MVI A");
    break;
  case 0x56: // NOLINT
    printf("MOV D,M");
    break;
  case 0x5e: // NOLINT
    printf("MOV E,M");
    break;
  case 0x66: // NOLINT
    printf("MOV H,M");
    break;
  case 0x6f: // NOLINT
    printf("MOV L,A");
    break;
  case 0x77: // NOLINT
    printf("MOV M,A");
    break;
  case 0x7a: // NOLINT
    printf("MOV A,D");
    break;
  case 0x7b: // NOLINT
    printf("MOV A,E");
    break;
  case 0x7c: // NOLINT
    printf("MOV A,H");
    break;
  case 0x7e: // NOLINT
    printf("MOV A,M");
    break;
  case 0xa7: // NOLINT
    printf("ANA A");
    break;
  case 0xaf: // NOLINT
    printf("XRA A");
    break;
  case 0xc1: // NOLINT
    printf("POP B");
    break;
  case 0xc2: // NOLINT
    printf("JNZ ");
    break;
  case 0xc3: // NOLINT
    printf("JMP ");
    break;
  case 0xc5: // NOLINT
    printf("PUSH B");
    break;
  case 0xc6: // NOLINT
    printf("ADI ");
    break;
  case 0xc9: // NOLINT
    printf("RET");
    break;
  case 0xcd: // NOLINT
    printf("CALL ");
    break;
  case 0xd1: // NOLINT
    printf("POP D");
    break;
  case 0xd3: // NOLINT
    printf("OUT ");
    break;
  case 0xd5: // NOLINT
    printf("PUSH D");
    break;
  case 0xe1: // NOLINT
    printf("POP H");
    break;
  case 0xe5: // NOLINT
    printf("PUSH H");
    break;
  case 0xe6: // NOLINT
    printf("ANI ");
    break;
  case 0xeb: // NOLINT
    printf("XCHG");
    break;
  case 0xf1: // NOLINT
    printf("POP PSW");
    break;
  case 0xf5: // NOLINT
    printf("PUSH PSW");
    break;
  case 0xfb: // NOLINT
    printf("EI");
    break;
  case 0xfe: // NOLINT
    printf("CPI ");
    break;
  default:
    break;
  }
}

void
cpu_init(i8080 *cpu)
{
  cpu->a = 0;
  cpu->b = 0;
  cpu->c = 0;
  cpu->d = 0;
  cpu->e = 0;
  cpu->h = 0;
  cpu->l = 0;

  cpu->flags = 0;
  cpu->pc = 0;
  cpu->sp = 0;
  cpu->interrupt_enabled = false;
  cpu->halted = false;
}

uint8_t
cpu_read_mem(i8080 *cpu, uint16_t address)
{
  return cpu->memory[address];
}

void
cpu_write_mem(i8080 *cpu, uint16_t address, uint8_t data)
{
  cpu->memory[address] = data;
}

bool
cpu_load_file(i8080 *cpu, const char *file_path, uint16_t address)
{
  FILE *file = fopen(file_path, "rb");

  if (file == NULL)
    {
      printf("Error: Unable to open file %s\n", file_path);
      return false;
    }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (address + file_size > MEM_SIZE)
    {
      printf("Error: File size exceeds available memory\n");
      fclose(file);
      return false;
    }

  size_t bytes_read = fread(&cpu->memory[address], 1, file_size, file);
  fclose(file);

  if (bytes_read != file_size)
    {
      printf("Error: Unable to read the entire file into memory\n");
      return false;
    }

  return true;
}

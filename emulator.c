#include "emulator.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Execute Instruction
int
execute_instruction(i8080 *cpu, uint8_t opcode)
{
  switch (opcode)
    {
    case 0x00: // NOLINT
      // printf("NOP");
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
      uint32_t bc = (cpu->b << 8) | (cpu->c);
      uint32_t hl = (cpu->h << 8) | (cpu->l);
      uint32_t result = hl + bc;
      update_carry_flag(cpu, result > 0xFF);
      cpu->h = (result & 0xff00) >> 8;
      cpu->l = (result & 0xff);
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
      cpu->d = cpu_read_mem(cpu, cpu->pc + 1);
      cpu->e = cpu_read_mem(cpu, cpu->pc + 2);
      cpu->pc += 2;
      break;
    case 0x13: // NOLINT
      {        // INX D
        cpu->e += 1;

        if (cpu->e == 0)
          {
            cpu->d += 1;
          }

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
      cpu->h = cpu_read_mem(cpu, cpu->pc + 1);
      cpu->l = cpu_read_mem(cpu, cpu->pc + 2);
      cpu->pc += 2;
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
      cpu->sp = cpu_read_mem(cpu, cpu->pc + 1) | (cpu_read_mem(cpu, cpu->pc + 2) << 8);
      cpu->pc += 2;
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
      cpu->a = cpu_read_mem(cpu, cpu->pc + 1);
      cpu->pc += 1;
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
      cpu->l = cpu->a;
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
      cpu->a = cpu->h;
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
      cpu->c = cpu->memory[cpu->sp];
      cpu->b = cpu->memory[cpu->sp + 1];
      cpu->sp += 2;
      cpu->pc += 1;
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
      // Affects Z, S, P, CY, AC
      uint8_t immediate = cpu_read_mem(cpu, cpu->pc + 1);
      uint16_t result = cpu->a + immediate;
      update_zero_flag(cpu, (uint8_t)result);
      update_sign_flag(cpu, (uint8_t)result);
      update_parity_flag(cpu, (uint8_t)result);
      update_carry_flag(cpu, result > 0xFF);
      update_aux_carry_flag(cpu, cpu->a, immediate);
      cpu->a = (uint8_t)result;
      cpu->pc += 2;
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
      // cpu->a will be byte one and port to write to is port 2
      uint8_t port = cpu_read_mem(cpu, cpu->pc + 1);
      cpu->pc += 2;
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
      immediate = cpu_read_mem(cpu, cpu->pc + 1);
      cpu->a &= immediate;
      update_zero_flag(cpu, cpu->a);
      update_sign_flag(cpu, cpu->a);
      update_parity_flag(cpu, cpu->a);
      cpu->flags |= FLAG_CY;
      cpu->flags |= FLAG_AC;
      cpu->pc += 2;
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
      cpu->interrupt_enabled = true;
      cpu->pc += 1;
      break;
    case 0xfe: // NOLINT
      printf("CPI ");
      break;
    default:
      {
        fprintf(stderr, "Error: opcode 0x%02x not found\n", opcode);
        return -1;
      }
    }
  cpu->pc += 1;
  return 0;
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
      fprintf(stderr, "Error: Unable to open file %s\n", file_path);
      return false;
    }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);

  if (file_size < 0)
    {
      perror("Error: unable to obtain file size");
      return false;
    }

  fseek(file, 0, SEEK_SET);

  if (address + file_size > MEM_SIZE)
    {
      fprintf(stderr, "Error: File size exceeds available memory\n");
      fclose(file);
      return false;
    }

  size_t bytes_read = fread(&cpu->memory[address], 1, file_size, file);
  fclose(file);

  if (bytes_read != file_size)
    {
      fprintf(stderr, "Error: Unable to read the entire file into memory\n");
      return false;
    }

  return true;
}

// FLAGS

void
update_aux_carry_flag(i8080 *cpu, uint8_t a, uint8_t b)
{
  // Masks highest 4 bits which preserves the nibbles(last 4 bits of a and b)
  // then adds nibbles to test for AC
  uint16_t result = (a & 0x0F) + (b & 0x0F); // NOLINT
  if (result & 0x10)                         // NOLINT
    { // Check if carry from bit 3 to bit 4 existss
      cpu->flags |= FLAG_AC;
    }
  else
    {
      cpu->flags &= ~FLAG_AC;
    }
}

void
update_zero_flag(i8080 *cpu, uint8_t result)
{
  if (result == 0)
    {
      cpu->flags |= FLAG_Z;
    }
  else
    {
      cpu->flags &= ~FLAG_Z;
    }
}

void
update_carry_flag(i8080 *cpu, bool carry_occurred)
{
  if (carry_occurred)
    {
      cpu->flags |= FLAG_CY;
    }
  else
    {
      cpu->flags &= ~FLAG_CY;
    }
}
int
count_set_bits(uint8_t value)
{
  int count = 0;
  for (int i = 0; i < 8; i++) // NOLINT
    {
      if (value & (1 << i))
        {
          count++;
        }
    }
  return count;
}

void
update_parity_flag(i8080 *cpu, uint8_t result)
{
  int set_bits = count_set_bits(result);
  if (set_bits % 2 == 0)
    {
      cpu->flags |= FLAG_P;
    }
  else
    {
      cpu->flags &= ~FLAG_P;
    }
}
bool
is_sign_flag_set(i8080 *cpu)
{
  return (cpu->flags & FLAG_S) != 0;
}

void
update_sign_flag(i8080 *cpu, uint8_t result)
{
  if (result & 0x80) // NOLINT
    {
      cpu->flags |= FLAG_S;
    }
  else
    {
      cpu->flags &= ~FLAG_S;
    }
}

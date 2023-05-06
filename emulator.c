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
      printf("NOP");
      break;
    case 0x01: // NOLINT
      printf("LXI B");
      break;
    case 0x05: // NOLINT
      printf("DCR B");
      break;
    case 0x06: // NOLINT
      { // MVI B, mem8
        cpu->b = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->pc += 1;
        break;
      }
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
      { // RRC  

	// keep bit 0
	uint8_t tmp = cpu->a << 7;
        
	// right shift register a
	cpu->a = cpu->a >> 1;

	// replace wrapped bit & set cy flag
	if (cpu->a ^ tmp)
	{
          cpu->a = (cpu->a ^ tmp); 
          cpu->flags = (cpu->flags | FLAG_CY); 
        }
	else // set cy flag to 0
	{
	  cpu->flags = (cpu->flags & 0x1F7); // NOLINT
	}
	
        break;
      }
    case 0x11: // NOLINT
      printf("LXI D");
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
      { // LDAX D
        // get addr
	uint16_t addr = cpu->e;
	addr = addr << 8; 
	addr += cpu->d;

        // get value at addr
        uint8_t val = cpu_read_mem(cpu, addr);

        // put value in d
        cpu->d = val;  
        break;
      }
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
      { // DAD H
        uint32_t sum = cpu->l;
        sum = (sum << 8) + cpu->h;
        sum = sum << 1; // double hl

        // update carry flag
        update_carry_flag(cpu, sum > 0xffff); // NOLINT

	// put values back in registers
        cpu->h = sum;
        cpu->l = (sum >> 8); 
        break;
      }
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
      { // LDA adr
        uint16_t addr = cpu_read_mem(cpu, cpu->pc + 2);
        addr = (addr << 8) + cpu_read_mem(cpu, cpu->pc + 1); 
        cpu->a = cpu_read_mem(cpu, addr); 
	cpu->pc += 2;
        break;
      }
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
      { // MOV H,M
        uint16_t addr = cpu->l;  
        addr = (addr << 8) + cpu->h; // NOLINT
        cpu->h = cpu_read_mem(cpu, addr);
        break;
      }
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
      { // MOV A,E 
        cpu->a = cpu->e; 	
        break;
      }
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
      { // TODO XRA A
        uint8_t result = cpu->a ^ cpu->a;
	update_sign_flag(cpu, result);
	update_zero_flag(cpu, result);
	update_parity_flag(cpu, result);
	update_carry_flag(cpu, false);
	update_aux_carry_flag(cpu, result, 0xFF);

	cpu->a = result;
        break;
      }
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
      // TODO
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
      // TODO
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
      // TODO
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
      // TODO
      printf("PUSH PSW");
      break;
    case 0xfb: // NOLINT
      printf("EI");
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

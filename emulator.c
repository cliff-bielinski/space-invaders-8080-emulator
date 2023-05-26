#include "emulator.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BYTE 8
#define UPPER_8_BIT_MASK 0xFF00
#define LOWER_8_BIT_MASK 0x00FF
#define RST_RANGE 7

// Execute Instruction
int
execute_instruction(i8080 *cpu, uint8_t opcode)
{
  int num_cycles = 0;
  switch (opcode)
    {
    case 0x00:          // NOLINT
      {                 // NOP
        num_cycles = 4; // NOLINT
        break;
      }
    case 0x01: // NOLINT
      {        // LXI B
        cpu->c = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->b = cpu_read_mem(cpu, cpu->pc + 2);
        cpu->pc += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x05: // NOLINT
      {        // DCR B
        cpu->b -= 1;

        update_zero_flag(cpu, cpu->b);
        update_sign_flag(cpu, cpu->b);
        update_parity_flag(cpu, cpu->b);
        update_aux_carry_flag(cpu, cpu->b, 0xFF); // NOLINT
        num_cycles = 5;                           // NOLINT

        break;
      }
    case 0x06: // NOLINT
      {        // MVI B, mem8
        cpu->b = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x09: // NOLINT
      // DAD B
      {
        uint32_t bc = (cpu->b << 8) | (cpu->c); // NOLINT
        uint32_t hl = (cpu->h << 8) | (cpu->l); // NOLINT
        uint32_t result = hl + bc;
        update_carry_flag(cpu, result > 0xFFFF); // NOLINT
        cpu->h = (result & 0xff00) >> 8;         // NOLINT
        cpu->l = (result & 0xff);                // NOLINT
        num_cycles = 10;                         // NOLINT
        break;
      }
    case 0x0a: // NOLINT
      {        // LDAX B
               // get addr
        uint16_t addr = cpu->b;
        addr = addr << 8; // NOLINT
        addr += cpu->c;

        // get value at addr
        uint8_t val = cpu_read_mem(cpu, addr);

        // put value in a
        cpu->a = val;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x0d: // NOLINT
      {        // DCR C
        uint8_t result = cpu->c - 1;
        update_zero_flag(cpu, result);
        update_sign_flag(cpu, result);
        update_parity_flag(cpu, result);
        update_aux_carry_flag(cpu, cpu->c, 0xFF); // NOLINT
        cpu->c = result;
        num_cycles = 5; // NOLINT
        break;
      }
    case 0x0e: // NOLINT
      {        // MVI C, D8
        cpu->c = cpu_read_mem(cpu, (cpu->pc + 1));
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x0f: // NOLINT
      {        // RRC

        // keep bit 0
        uint8_t tmp = cpu->a << 7; // NOLINT

        // right shift register a
        cpu->a = cpu->a >> 1;

        // replace wrapped bit
        cpu->a ^= tmp;

        // set cy flag to prev bit 0
        if (tmp != 0)
          {
            update_carry_flag(cpu, true);
          }
        else
          {
            update_carry_flag(cpu, false);
          }
        num_cycles = 4; // NOLINT
        break;
      }
    case 0x11: // NOLINT
      {
        cpu->e = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->d = cpu_read_mem(cpu, cpu->pc + 2);
        cpu->pc += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x13: // NOLINT
      {        // INX D
        cpu->e += 1;
        if (cpu->e == 0)
          {
            cpu->d += 1;
          }
        break;
        num_cycles = 5; // NOLINT
      }
    case 0x19: // NOLINT
      {        // DAD D
        uint32_t sum
            = (cpu->h << 8) // NOLINT
              | (cpu->l);   // convert reg pair h,l to 16 bit int // NOLINT

        // Convert rep pair d,e to 16 bit int.
        uint32_t reg_de = (cpu->d << 8) | (cpu->e); // NOLINT

        sum += reg_de;

        cpu->h = sum >> 8;                      // NOLINT
        cpu->l = sum & 0xFF;                    // NOLINT
        update_carry_flag(cpu, (sum > 0xFFFF)); // NOLINT
        num_cycles = 10;                        // NOLINT

        break;
      }
    case 0x1a: // NOLINT
      {        // LDAX D
               // get addr
        uint16_t addr = cpu->d;
        addr = addr << 8; // NOLINT
        addr += cpu->e;

        // get value at addr
        uint8_t val = cpu_read_mem(cpu, addr);

        // put value in a
        cpu->a = val;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x21: // NOLINT
      // LXI H
      {
        cpu->l = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->h = cpu_read_mem(cpu, cpu->pc + 2);
        cpu->pc += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x23: // NOLINT
      {        // INX H
        cpu->l += 1;
        if (cpu->l == 0)
          {
            cpu->h += 1;
          }
        num_cycles = 5; // NOLINT
        break;
      }
    case 0x26: // NOLINT
      {        // MVI H, D8
        cpu->h = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x29: // NOLINT
      {        // DAD H
        uint32_t sum = cpu->h;
        sum = (sum << 8) + cpu->l; // NOLINT
        sum = sum << 1;            // double hl

        // update carry flag
        update_carry_flag(cpu, sum > 0xffff); // NOLINT

        // put values back in registers
        cpu->l = sum;
        cpu->h = (sum >> 8); // NOLINT
        num_cycles = 10;     // NOLINT
        break;
      }
    case 0x31: // NOLINT
      {
        // NOLINTNEXTLINE
        cpu->sp = cpu_read_mem(cpu, cpu->pc + 1)
                  | (cpu_read_mem(cpu, cpu->pc + 2) << 8); // NOLINT
        cpu->pc += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x32: // NOLINT
      {        // STA
        // little endian - first byte is LSB, second byte is MSB for memory
        // address
        uint16_t address = cpu_read_mem(cpu, cpu->pc + 1);
        address += (cpu_read_mem(cpu, cpu->pc + 2) << 8); // NOLINT
        cpu_write_mem(cpu, address, cpu->a);
        cpu->pc += 2;
        num_cycles = 13; // NOLINT
        break;
      }
    case 0x35: // NOLINT
      {        // DCR M
        uint16_t address = cpu->l;
        address += (cpu->h << 8); // NOLINT
        uint8_t mem_value = cpu_read_mem(cpu, address);
        uint8_t result = mem_value - 1;
        update_zero_flag(cpu, result);
        update_sign_flag(cpu, result);
        update_parity_flag(cpu, result);
        update_aux_carry_flag(cpu, mem_value, 0xFF); // NOLINT
        cpu_write_mem(cpu, address, result);
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x36: // NOLINT
      {        // MVI M, D8
               // Address is stored in reg h,l.
        uint16_t address = (cpu->h << 8) | cpu->l; // NOLINT

        cpu_write_mem(cpu, address, cpu_read_mem(cpu, cpu->pc + 1));

        cpu->pc += 1;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x37: // NOLINT
      {        // STC
        cpu->flags |= FLAG_CY;
        num_cycles = 4; // NOLINT
        break;
      }
    case 0x3a: // NOLINT
      {        // LDA adr
        uint16_t addr = cpu_read_mem(cpu, cpu->pc + 2);
        addr = (addr << 8) + cpu_read_mem(cpu, cpu->pc + 1); // NOLINT
        cpu->a = cpu_read_mem(cpu, addr);
        cpu->pc += 2;
        num_cycles = 13; // NOLINT
        break;
      }
    case 0x3d: // NOLINT
      {        // DCR A
        cpu->a -= 1;

        update_zero_flag(cpu, cpu->a);
        update_sign_flag(cpu, cpu->a);
        update_parity_flag(cpu, cpu->a);
        update_aux_carry_flag(cpu, cpu->a, 0xFF); // NOLINT
        num_cycles = 5;                           // NOLINT

        break;
      }
    case 0x3e: // NOLINT
      {
        cpu->a = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x56: // NOLINT
      {        // MOV D,M
        // 16-bit memory address located in registers HL
        uint16_t address = cpu->l;
        address += (cpu->h << 8); // NOLINT
        cpu->d = cpu_read_mem(cpu, address);
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x5e: // NOLINT
      {        // MOV E,M
        // Address is stored in reg h,l.
        uint16_t address = (cpu->h << 8) | cpu->l; // NOLINT

        cpu->e = cpu_read_mem(cpu, address);
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x66: // NOLINT
      {        // MOV H,M
        uint16_t addr = cpu->h;
        addr = (addr << 8) + cpu->l; // NOLINT
        cpu->h = cpu_read_mem(cpu, addr);
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x6f: // NOLINT
      {
        cpu->l = cpu->a;
        num_cycles = 5; // NOLINT
        break;
      }
    case 0x77: // NOLINT
      {        // MOV M,A
        uint16_t address = cpu->l;
        address += (cpu->h << 8); // NOLINT
        cpu_write_mem(cpu, address, cpu->a);
        num_cycles = 7; // NOLINT
        break;
      }
    case 0x7a: // NOLINT
      {        // MOV A,D
        cpu->a = cpu->d;
        num_cycles = 5; // NOLINT
        break;
      }
    case 0x7b: // NOLINT
      {        // MOV A,E
        cpu->a = cpu->e;
        num_cycles = 5; // NOLINT
        break;
      }
    case 0x7c: // NOLINT
      {
        cpu->a = cpu->h;
        num_cycles = 5; // NOLINT
        break;
      }
    case 0x7e: // NOLINT
      {        // MOV A,M
        uint16_t address = cpu->l;
        address += (cpu->h << 8); // NOLINT
        cpu->a = cpu_read_mem(cpu, address);
        num_cycles = 7; // NOLINT
        break;
      }
    case 0xa7: // NOLINT
      {        // ANA A
        uint8_t temp = cpu->a;
        cpu->a = cpu->a & cpu->a;

        update_zero_flag(cpu, cpu->a);
        update_sign_flag(cpu, cpu->a);
        update_parity_flag(cpu, cpu->a);
        update_aux_carry_flag(cpu, temp, cpu->a);
        update_carry_flag(cpu, false);
        num_cycles = 4; // NOLINT

        break;
      }
    case 0xaf: // NOLINT
      {        // XRA A
        uint8_t result = cpu->a ^ cpu->a;
        update_sign_flag(cpu, result);
        update_zero_flag(cpu, result);
        update_parity_flag(cpu, result);
        update_carry_flag(cpu, false);
        update_aux_carry_flag(cpu, result, 0xFF); // NOLINT

        cpu->a = result;
        num_cycles = 4; // NOLINT

        break;
      }
    case 0xc1: // NOLINT
      {
        cpu->c = cpu_read_mem(cpu, cpu->sp);
        cpu->b = cpu_read_mem(cpu, cpu->sp + 1);
        cpu->sp += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xc2: // NOLINT
      {        // JNZ
        uint16_t address = cpu_read_mem(cpu, cpu->pc + 1);
        address += (cpu_read_mem(cpu, cpu->pc + 2) << 8); // NOLINT
        if ((cpu->flags & FLAG_Z) == 0)
          {
            // returns rather than breaks to avoid pc increment at end of
            // function
            cpu->pc = address;
            // NOLINTNEXTLINE
            return 10; // return cycle number
          }
        cpu->pc += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xc3: // NOLINT
      {        // JMP
        // address format is instruction byte 3 byte 2 little endian.
        uint16_t address = cpu_read_mem(cpu, cpu->pc + 2);
        address = (address << 8) | cpu_read_mem(cpu, cpu->pc + 1); // NOLINT
        cpu->pc = address;
        // NOLINTNEXTLINE
        return 10; // no PC increment due to JMP, return num cycles
      }
    case 0xc5: // NOLINT
      {        // PUSH B
        cpu_write_mem(cpu, cpu->sp - 1, cpu->b);
        cpu->sp -= 1;
        cpu_write_mem(cpu, cpu->sp - 1, cpu->c);
        cpu->sp -= 1;
        num_cycles = 11; // NOLINT
        break;
      }
    case 0xc6: // NOLINT
      {
        // Affects Z, S, P, CY, AC
        uint8_t immediate = cpu_read_mem(cpu, cpu->pc + 1);
        uint16_t answer = cpu->a + immediate;
        update_zero_flag(cpu, (uint8_t)answer);
        update_sign_flag(cpu, (uint8_t)answer);
        update_parity_flag(cpu, (uint8_t)answer);
        update_carry_flag(cpu, answer > 0xFF); // NOLINT
        update_aux_carry_flag(cpu, cpu->a, immediate);
        cpu->a = (uint8_t)answer;
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0xc8:                               // NOLINT
      {                                      // RZ
        if ((cpu->flags & FLAG_Z) == FLAG_Z) // if Z set, RET
          {
            uint16_t address = cpu_read_mem(cpu, cpu->sp);
            address += (cpu_read_mem(cpu, cpu->sp + 1) << 8); // NOLINT
            cpu->sp += 2;
            cpu->pc = address;
            // NOLINTNEXTLINE
            return 11; // return num cycles
          }
        num_cycles = 5; // NOLINT
        break;
      }
    case 0xc9: // NOLINT
      {        // RET
        // returns rather than breaks to avoid pc increment at end of function
        uint16_t address = cpu_read_mem(cpu, cpu->sp);
        address += (cpu_read_mem(cpu, cpu->sp + 1) << 8); // NOLINT
        cpu->sp += 2;
        cpu->pc = address;
        // NOLINTNEXTLINE
        return 10; // return num cycles
      }
    case 0xca: // NOLINT
      {        // JZ
        if (is_zero_flag_set(cpu))
          {
            uint16_t address = cpu_read_mem(cpu, cpu->pc + 2); // NOLINT
            address = address << 8;                            // NOLINT
            address += cpu_read_mem(cpu, cpu->pc + 1);
            cpu->pc = address;
            // NOLINTNEXTLINE
            return 10; // return num cycles
          }
        cpu->pc += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xcd:                                                   // NOLINT
      {                                                          // CALL ADDR
        cpu_write_mem(cpu, cpu->sp - 1, ((cpu->pc + 3) >> 8));   // NOLINT
        cpu_write_mem(cpu, cpu->sp - 2, ((cpu->pc + 3) & 0xFF)); // NOLINT

        cpu->sp -= 2;
        cpu->pc = (cpu_read_mem(cpu, cpu->pc + 2) << 8) // NOLINT
                  | (cpu_read_mem(cpu, cpu->pc + 1));
        // NOLINTNEXTLINE
        return 17; // return num cycles
      }
    case 0xd1: // NOLINT
      {
        // POP D
        cpu->e = cpu_read_mem(cpu, cpu->sp);
        cpu->sp += 1;
        cpu->d = cpu_read_mem(cpu, cpu->sp);
        cpu->sp += 1;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xd2: // NOLINT
      {        // JNC ADR
        uint16_t address = cpu_read_mem(cpu, cpu->pc + 2);
        address = (address << BYTE) | cpu_read_mem(cpu, cpu->pc + 1);
        if ((cpu->flags & FLAG_CY) != FLAG_CY) // if CY not set JUMP
          {
            cpu->pc = address;
            // NOLINTNEXTLINE
            return 10; // return cycle number
          }
        cpu->pc += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xd3: // NOLINT
      {
        // printf("OUT ");
        // cpu->a will be byte one and port to write to is port 2
        uint8_t port = cpu_read_mem(cpu, cpu->pc + 1);
        printf("%u", port);
        cpu->pc += 1;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xd5: // NOLINT
      {        // PUSH D
        cpu_write_mem(cpu, cpu->sp - 2, cpu->e);
        cpu_write_mem(cpu, cpu->sp - 1, cpu->d);
        cpu->sp -= 2;    // NOLINT
        num_cycles = 11; // NOLINT
        break;
      }
    case 0xda: // NOLINT
      {        // JC ADR
        uint16_t address = cpu_read_mem(cpu, cpu->pc + 2);
        address = (address << BYTE) | cpu_read_mem(cpu, cpu->pc + 1);
        if ((cpu->flags & FLAG_CY) == FLAG_CY) // if CY set JUMP
          {
            cpu->pc = address;
            // NOLINTNEXTLINE
            return 10; // return cycle number
          }
        cpu->pc += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xdb: // NOLINT
      {        // IN D8
        uint8_t port = cpu_read_mem(cpu, cpu->pc + 1);
        printf("%u", port);
        cpu->pc += 1;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xe1: // NOLINT
      {        // POP H
        cpu->l = cpu_read_mem(cpu, cpu->sp);
        cpu->h = cpu_read_mem(cpu, cpu->sp + 1);
        cpu->sp += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xe5: // NOLINT
      {        // PUSH H
        cpu_write_mem(cpu, cpu->sp - 1, cpu->h);
        cpu->sp -= 1;
        cpu_write_mem(cpu, cpu->sp - 1, cpu->l);
        cpu->sp -= 1;
        num_cycles = 11; // NOLINT
        break;
      }
    case 0xe6: // NOLINT
      {
        uint8_t immediate = cpu_read_mem(cpu, cpu->pc + 1);
        cpu->a &= immediate;
        update_zero_flag(cpu, cpu->a);
        update_sign_flag(cpu, cpu->a);
        update_parity_flag(cpu, cpu->a);
        update_carry_flag(cpu, false);
        cpu->flags &= ~FLAG_AC;
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0xeb: // NOLINT
      {        // XCHG
        // exchange h and d
        uint8_t temp = cpu->h;
        cpu->h = cpu->d;
        cpu->d = temp;

        // exchange l and e
        temp = cpu->l;
        cpu->l = cpu->e;
        cpu->e = temp;
        num_cycles = 5; // NOLINT
        break;
      }
    case 0xf1: // NOLINT
      {        // POP PSW
        cpu->flags = cpu_read_mem(cpu, cpu->sp);
        cpu->a = cpu_read_mem(cpu, cpu->sp + 1);
        cpu->sp += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xf5: // NOLINT
      {        // PUSH PSW
        cpu_write_mem(cpu, cpu->sp - 1, cpu->a);
        cpu->sp -= 1;
        cpu_write_mem(cpu, cpu->sp - 1, cpu->flags);
        cpu->sp -= 1;
        num_cycles = 11; // NOLINT
        break;
      }
      break;
    case 0xfb: // NOLINT
      {
        cpu->interrupt_enabled = true;
        num_cycles = 4; // NOLINT
        break;
      }
    case 0xfe: // NOLINT
      {        // CPI
        uint8_t data = cpu_read_mem(cpu, cpu->pc + 1);
        uint8_t result = cpu->a - data;
        update_zero_flag(cpu, result);
        update_sign_flag(cpu, result);
        update_parity_flag(cpu, result);
        update_carry_flag(cpu, (data > cpu->a));
        update_aux_carry_flag(cpu, cpu->a, (~data + 1));
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    default:
      {
        fprintf(stderr, "Error: opcode 0x%02x not found\n", opcode);
        return -1;
      }
    }
  cpu->pc += 1;
  return num_cycles;
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
    { // Check if carry from bit 3 to bit 4 exists
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

bool
is_zero_flag_set(i8080 *cpu)
{
  return (cpu->flags & FLAG_Z) != 0;
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

// Interrupts


// UPDATE GRAPHICS

void
update_graphics(i8080 *cpu, SDL_Surface *buffer, SDL_Surface *surface)
{

  uint32_t *screen_buff = buffer->pixels;

  // Graphics data is rotated 90 degrees in memory counter-clockwise.  Reading
  // byte by byte starting at 0x2400 we need to fill in the screen left to
  // right, bottom to top.
  int vram = 0x2400;
  // Start at the left edge
  for (int column = 0; column > SCREEN_WIDTH; column++)
    {
      // Start at bottom of screen, decrement by 8 since each bit is a pixel.
      for (int row = SCREEN_HEIGHT; row > 0; row -= 8) // NOLINT
        {
          // Set each pixel based on bit value.
          for (int pixel = 0; pixel < 8; pixel++) // NOLINT
            {
              // Calculate surface index.
              int surf_index = (SCREEN_WIDTH * (row - pixel)) + column
                               - (SCREEN_WIDTH - 1);

              uint8_t cur_byte = cpu_read_mem(cpu, vram);

              // Set pixel to on by changing color to white.
              if (cur_byte & 1 << pixel)
                {
                  screen_buff[surf_index] = 0xFFFFFF; // NOLINT
                }
              else
                {
                  // Set pixel to off by changing color to black.
                  screen_buff[surf_index] = 0x000000; // NOLINT
                }
            }
        }
      vram++; // Increment to next byte in VRAM
    }

  SDL_BlitSurface(buffer, NULL, surface, NULL);
}

// DEBUGGING FUNCTIONS

void
print_instruction(uint8_t opcode)
{
  printf("INSTRUCTION: 0x%02x\n", opcode);
}

void
print_state(i8080 *cpu)
{
  printf("REGISTERS a: 0x%02x b: 0x%02x c: 0x%02x d: 0x%02x e: 0x%02x h: "
         "0x%02x l: 0x%02x sp: 0x%04x pc: 0x%04x ",
         cpu->a, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l, cpu->sp,
         cpu->pc);
}

void
print_flags(uint8_t flags)
{
  printf("FLAGS z: %d s: %d p: %d cy: %d ac %d", (flags & FLAG_Z) == FLAG_Z,
         (flags & FLAG_S) == FLAG_S, (flags & FLAG_P) == FLAG_P,
         (flags & FLAG_CY) == FLAG_CY, (flags & FLAG_AC) == FLAG_AC);
}

// INTERRUPTS
int
handle_interrupt(i8080 *cpu, uint8_t rst_instruction)
{
  if (rst_instruction > RST_RANGE)
    {
      fprintf(stderr, "Invalid restart instruction %u", rst_instruction);
      return -1;
    }

  if (cpu->interrupt_enabled == false)
    {
      return 0;
    }

  // get address for interrupt subroutine
  uint16_t subroutine_address = BYTE * rst_instruction;

  // push program counter to stack
  cpu_write_mem(cpu, cpu->sp - 1,
                (uint8_t)((cpu->pc & UPPER_8_BIT_MASK) >> BYTE));
  cpu_write_mem(cpu, cpu->sp - 2, (uint8_t)(cpu->pc & LOWER_8_BIT_MASK));
  cpu->sp -= 2;

  // set program counter to start of the interrupt subroutine
  cpu->pc = subroutine_address;

  // disable interrupts
  cpu->interrupt_enabled = false;

  return 0;
}

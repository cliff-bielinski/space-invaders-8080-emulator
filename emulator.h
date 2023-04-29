#include <stdbool.h>
#include <stdint.h>


// Flags Defined
#define FLAG_S 0x80
#define FLAG_Z 0x40
#define FLAG_AC 0x20
#define FLAG_P 0x10
#define FLAG_CY 0x08

// Memory
#define MEM_SIZE 65536

typedef struct
{
  // Registers
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t h;
  uint8_t l;

  // Flags (stored in the F register)
  // Includes z, s, p, cy, ac, pad

  uint8_t flags;

  /*
  Sign (S): Set if the result of an operation is negative (most significant bit
  is 1). Zero (Z): Set if the result of an operation is zero. Auxiliary Carry
  (AC): Set if there's a carry from bit 3 to bit 4 during an operation. Parity
  (P): Set if the result of an operation has an even number of 1 bits. Carry
  (CY): Set if there's a carry from the most significant bit after an
  operation.
  */

  // Program Counter & Stack pointer

  uint16_t pc, sp;

  // Memory

  uint8_t memory[MEM_SIZE];

  // Internal state for interrupt tracking and halted status tracking

  bool interrupt_enabled;
  bool halted;

} i8080;

// Funct prototypes
void cpu_init(i8080 *cpu);
uint8_t cpu_read_mem(i8080 *cpu, uint16_t address);
void cpu_write_mem(i8080 *cpu, uint16_t address, uint8_t data);
bool cpu_load_file(i8080 *cpu, const char *file_path, uint16_t address);
void execute_instruction(i8080 *cpu, uint8_t opcode);

// Prototypes for Flags
// Auxiliary Carry (AC)
void update_aux_carry_flag(i8080 *cpu, uint8_t a, uint8_t b);

// Zero Flag
void update_zero_flag(i8080 *cpu, uint8_t result);

// Sign Flag
void update_sign_flag(i8080 *cpu, uint8_t result);
bool is_sign_flag_set(i8080 *cpu);
// Carry Flag 
void update_carry_flag(i8080 *cpu, bool carry_occurred);
/*
How to use in a function
update_carry_flag(cpu, result > 0xFF); result > 0XFF will return true or false
*/

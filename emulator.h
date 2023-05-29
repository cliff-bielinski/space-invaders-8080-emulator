#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// SDL Files
#include <SDL2/SDL.h>

// #include "SDL_nmix.h"
// #include "SDL_nmix_file.h"

// Flags Defined
#define FLAG_S 0x80  // NOLINT
#define FLAG_Z 0x40  // NOLINT
#define FLAG_AC 0x20 // NOLINT
#define FLAG_P 0x10  // NOLINT
#define FLAG_CY 0x08 // NOLINT

// Register Pairs
#define PSW 0
#define BC 1
#define DE 2
#define HL 3
#define SP 4
#define PC 5

// Memory
#define MEM_SIZE 65536 // NOLINT

// Display
#define SCREEN_WIDTH 224  // NOLINT
#define SCREEN_HEIGHT 256 // NOLINT

// Bit Manipulation
#define NIBBLE 4
#define BYTE 8
#define UPPER_4_BIT_MASK 0xF0
#define LOWER_4_BIT_MASK 0x0F
#define UPPER_8_BIT_MASK 0xFF00
#define LOWER_8_BIT_MASK 0x00FF
#define MAX_8_BIT_VALUE 0xFF
#define MAX_16_BIT_VALUE 0xFFFF

// Opcodes
#define RST_RANGE 7

typedef struct
{
  // Registers
  uint8_t a, b, c, d, e, h, l;

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

  bool colored_screen;
  // Ports & Shift registers for in/out opcode
  uint8_t port1, port2;
  uint8_t shift_msb, shift_lsb, shift_offset;
  uint8_t last_out_port3, last_out_port5;

} i8080;

// Funct prototypes
void cpu_init(i8080 *cpu);
uint8_t cpu_read_mem(i8080 *cpu, uint16_t address);
void cpu_write_mem(i8080 *cpu, uint16_t address, uint8_t data);
bool cpu_load_file(i8080 *cpu, const char *file_path, uint16_t address);
int execute_instruction(i8080 *cpu, uint8_t opcode);
void update_graphics(i8080 *cpu, SDL_Surface *buffer, SDL_Surface *surface);
void writeRegisterPair(i8080 *cpu, int pair, uint16_t value);
uint16_t readRegisterPair(i8080 *cpu, int pair);
uint8_t getImmediate8BitValue(i8080 *cpu);
uint16_t getImmediate16BitValue(i8080 *cpu);

// Prototypes for Flags

// Parity Flag
int count_set_bits(uint8_t value);
void update_parity_flag(i8080 *cpu, uint8_t result);

// Auxiliary Carry (AC)
void update_aux_carry_flag(i8080 *cpu, uint8_t a, uint8_t b);

// Zero Flag
void update_zero_flag(i8080 *cpu, uint8_t result);

// Sign Flag
void update_sign_flag(i8080 *cpu, uint8_t result);
bool is_sign_flag_set(i8080 *cpu);
bool is_zero_flag_set(i8080 *cpu);

// Carry Flag
void update_carry_flag(i8080 *cpu, bool carry_occurred);
/*
How to use in a function
update_carry_flag(cpu, result > 0xFF); result > 0XFF will return true or false
*/

void cpu_set_flag(i8080 *cpu, uint8_t flag, bool value);
bool cpu_get_flag(i8080 *cpu, uint8_t flag);

/*
Helper functions to print instruction and cpu state
*/
void print_instruction(uint8_t opcode);
void print_state(i8080 *cpu);
void print_flags(uint8_t flags);

/*
Interrupt functions
*/
int handle_interrupt(i8080 *cpu, uint8_t rst_instruction);

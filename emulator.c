#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE 65536

typedef struct {
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
  Sign (S): Set if the result of an operation is negative (most significant bit is 1).
  Zero (Z): Set if the result of an operation is zero.
  Auxiliary Carry (AC): Set if there's a carry from bit 3 to bit 4 during an operation.
  Parity (P): Set if the result of an operation has an even number of 1 bits.
  Carry (CY): Set if there's a carry from the most significant bit after an operation.
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

// Prototypes for Flags

void cpu_set_flag(i8080 *cpu, uint8_t flag, bool value);
bool cpu_get_flag(i8080 *cpu, uint8_t flag);


// Execute Instruction

void execute_instruction(i8080 *cpu, uint8_t opcode) {
  switch (opcode){
    ;

  }

}
void cpu_init(i8080 *cpu) {
  cpu -> a = 0;
  cpu -> b = 0;
  cpu -> c = 0;
  cpu -> d = 0;
  cpu -> e = 0;
  cpu -> h = 0;
  cpu -> l = 0;

  cpu -> flags = 0;
  cpu -> pc = 0;
  cpu -> sp = 0;
  cpu -> interrupt_enabled = false;
  cpu -> halted = false;

}

uint8_t cpu_read_mem(i8080 *cpu, uint16_t address){
  return cpu->memory[address];

}

void cpu_write_mem(i8080 *cpu, uint16_t address, uint8_t data) {
  cpu->memory[address] = data;
}
bool cpu_load_file(i8080 *cpu, const char *file_path, uint16_t address){

  FILE *file = fopen(file_path, "rb");
  if (file == NULL){
      printf("Error: Unable to open file %s\n", file_path);
      return false;
  }
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (address + file_size > MEM_SIZE) {
    printf("Error: File size exceeds available memory\n");
    fclose(file);
    return false;
  }
  size_t bytes_read = fread(&cpu->memory[address], 1, file_size, file);
  fclose(file);

  if (bytes_read != file_size){
    printf("Error: Unable to read the entire file into memory\n");
    return false;
  }
  return true;
}

int main() {
  i8080 cpu;
  cpu_init(&cpu);
  const char *rom_path = "./invaders";
  uint16_t load_address = 0x0000;
  // Load ROM
  if (!cpu_load_file(&cpu, rom_path, load_address)){
    printf("Failed to load ROM\n");
    return 1;

  }

  while (true) {

    // 1 Fetch, decode, and execute next instruction
    // fetch_decode_execute(&cpu)

    // 2 Handle interrupts
    // handle_interrupts(&cpu)

    // 3 Update system state for display, input, and sound

    // 4 Check for exit conditions

  }

  // Clean up resources and exit
}


















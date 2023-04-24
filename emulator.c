#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "emulator.h"

// Execute Instruction
void execute_instruction(i8080 *cpu, uint8_t opcode) {
  switch (opcode) {
    case 0x13: {    //INX D
      cpu->e += 1;

      if (cpu->e == 0) {
        cpu->d += 1;
      }
      
      cpu->pc += 1;
      break;
    }
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

uint8_t cpu_read_mem(i8080 *cpu, uint16_t address) {
  return cpu->memory[address];
}

void cpu_write_mem(i8080 *cpu, uint16_t address, uint8_t data) {
  cpu->memory[address] = data;
}

bool cpu_load_file(i8080 *cpu, const char *file_path, uint16_t address) {
  FILE *file = fopen(file_path, "rb");

  if (file == NULL){
      printf("Error: Unable to open file %s\n", file_path);
      return false;
  }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
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

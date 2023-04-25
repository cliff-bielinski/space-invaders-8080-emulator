#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "emulator.h"

int main() {
  i8080 cpu;
  cpu_init(&cpu);
  const char *rom_path = "./invaders";
  uint32_t load_address = 0x0000;
  // Load ROM
  size_t file_size = cpu_load_file(&cpu, rom_path, load_address);
  //printf("%zu\n", file_size);
  //printf("%lu\n", sizeof(uint32_t));
  if (file_size == 0) {
    printf("Failed to load ROM\n");
    return 1;
  }
  uint32_t end_address = load_address + file_size;
  print_memory(&cpu, load_address, end_address);
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

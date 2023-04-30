#include "emulator.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int
main()
{
  i8080 cpu;
  cpu_init(&cpu);
  const char *rom_path = "./invaders";
  // NOLINTNEXTLINE
  uint16_t load_address = 0x0000;
  // Load ROM
  if (!cpu_load_file(&cpu, rom_path, load_address))
    {
      printf("Failed to load ROM\n");
      return 1;
    }

  while (true)
    {
      exit(1);
      // 1 Fetch, decode, and execute next instruction
      // fetch_decode_execute(&cpu)

      // 2 Handle interrupts
      // handle_interrupts(&cpu)

      // 3 Update system state for display, input, and sound

      // 4 Check for exit conditions
    }

  // Clean up resources and exit
}
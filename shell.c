#include "emulator.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
  // Only accept one argument
  if (argc != 2)
    {
      fprintf(stderr, "Invalid number of arguments. Correct syntax: ./shell "
                      "rom_filepath\n");
      exit(EXIT_FAILURE);
    }

  // initialize CPU state
  i8080 cpu;
  cpu_init(&cpu);

  const char *rom_path = "./invaders";
  // NOLINTNEXTLINE

  uint16_t load_address = 0x0000;

  // Load ROM into memory
  if (!cpu_load_file(&cpu, argv[1], load_address))
    {
      fprintf(stderr, "Failed to load ROM\n");
      exit(EXIT_FAILURE);
    }

  while (true)
    {

      // 1 Fetch, decode, and execute next instruction
      // fetch_decode_execute(&cpu)

      // fetch and execute next instruction
      uint8_t next_instruction = cpu_read_mem(&cpu, cpu.pc);
      if (execute_instruction(&cpu, next_instruction) < 0)
        {
          fprintf(stderr,
                  "Unimplemented opcode encountered. Exiting program.\n");
          exit(EXIT_FAILURE);
        }

      // 2 Handle interrupts
      // handle_interrupts(&cpu)

      // 3 Update system state for display, input, and sound

      // 4 Check for exit conditions
    }

  // Clean up resources and exit
}
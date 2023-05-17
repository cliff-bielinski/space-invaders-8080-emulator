#include "emulator.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
  int pflag = 0;
  int dflag = 0;
  int opt;

  while ((opt = getopt(argc, argv, "pd")) != -1)
    {
      switch (opt)
        {
        case 'p':
          pflag = 1;
          break;
        case 'd':
          dflag = 1;
          break;
        case '?':
          if (isprint(optopt))
            {
              fprintf(stderr, "Unknown option '-%c'.\n", optopt);
            }
          else
            {
              fprintf(stderr, "Unknown option character '\\x%x.\n", optopt);
            }
          exit(EXIT_FAILURE);
        default:
          abort();
        }
    }

  // Only accept one non-option argument
  if ((argc - optind) != 1)
    {
      fprintf(stderr, "Invalid number of arguments. Program only takes "
                      "one non-option argument (rom_filepath).\n");
      exit(EXIT_FAILURE);
    }

  // initialize CPU state
  i8080 cpu;
  cpu_init(&cpu);

  // NOLINTNEXTLINE
  uint16_t load_address = 0x0000;

  // Load ROM into memory
  if (!cpu_load_file(&cpu, argv[optind], load_address))
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
      if (pflag)
        {
          print_instruction(next_instruction);
        }
      if (dflag)
        {
          printf("PRE-INSTRUCTION  ");
          print_state(&cpu);
          print_flags(cpu.flags);
          printf("\n");
        }
      if (execute_instruction(&cpu, next_instruction) < 0)
        {
          fprintf(stderr,
                  "Unimplemented opcode encountered. Exiting program.\n");
          exit(EXIT_FAILURE);
        }
      if (dflag)
        {
          printf("POST-INSTRUCTION ");
          print_state(&cpu);
          print_flags(cpu.flags);
          printf("\n");
        }

      // 2 Handle interrupts
      // handle_interrupts(&cpu)

      // 3 Update system state for display, input, and sound

      // 4 Check for exit conditions
    }

  // Clean up resources and exit
}

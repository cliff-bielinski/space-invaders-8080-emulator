#include "emulator.h"
#include <SDL2/SDL.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CLOCK_SPEED_MS 2000
#define(TICK 1000 * (1.0 / 60.0))
#define CYCLES_PER_TICK (CLOCK_SPEED_MS * TICK)

int run_cpu(i8080 *cpu, int cycles);
int pflag = 0;
int dflag = 0;
int opt;

int
main(int argc, char *argv[])
{

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

  // start timer
  uint64_t last_tick = SDL_GetTicks();

  // set initial offset value
  int cycle_offset = 0;
  int num_cycles = CYCLES_PER_TICK;

  while (true)
    {
      if ((SDL_GetTicks() - last_tick) > TICK) // NOLINT
        {
          printf("Current tick is: %d\n", SDL_GetTicks());
          // run first half of tick cycles
          cycle_offset = run_cpu(&cpu, (num_cycles - (abs(cycle_offset) / 2)));

          // first interrupt
          handle_interrupt(&cpu, 0x01);

          // run second half of tick cycles
          cycle_offset = run_cpu(&cpu, (num_cycles - (abs(cycle_offset) / 2)));

          // second interrupt
          handle_interrupt(&cpu, 0x02);

          // set number of cycles for next tick
          num_cycles = CYCLES_PER_TICK - cycle_offset;

          // 3 Update system state for display, input, and sound

          // 4 Check for exit conditions

          last_tick = SDL_GetTicks();
        }
    }
}

int
run_cpu(i8080 *cpu, int cycles)
{

  // fetch and execute next instruction
  while (cycles > 0)
    {
      uint8_t next_instruction = cpu_read_mem(cpu, cpu->pc);
      if (pflag)
        {
          print_instruction(next_instruction);
        }
      if (dflag)
        {
          printf("PRE-INSTRUCTION  ");
          print_state(cpu);
          print_flags(cpu->flags);
          printf("\n");
        }

      int num_cycles_used = execute_instruction(cpu, next_instruction);

      // execute instruction failed
      if (num_cycles_used < 0)
        {
          fprintf(stderr, "Unimplemented opcode encountered. "
                          "Exiting program.\n");
          exit(EXIT_FAILURE);
        }
      else
        {
          cycles -= num_cycles_used;
        }
      if (dflag)
        {
          printf("POST-INSTRUCTION ");
          print_state(cpu);
          print_flags(cpu->flags);
          printf("\n");
        }
    }
  return cycles;
}

#include "emulator.h"
#include <SDL2/SDL.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Screen dimension constants
uint32_t start_time = 0;
uint32_t last_interrupt = 0;

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

  // Render window
  SDL_Window *window = NULL;

  // The surface contained by the window
  SDL_Surface *screenSurface = NULL;
  SDL_Surface *buffer = NULL;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0)
    {
      printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
  else
    {
      // Create window
      window = SDL_CreateWindow("Space Invaders Emulator",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * 2,
                                SCREEN_HEIGHT * 2, SDL_WINDOW_RESIZABLE);
      if (window == NULL)
        {
          printf("Window could not be created! SDL_Error: %s\n",
                 SDL_GetError());
        }
      else
        {
          // Get window surface
          screenSurface = SDL_GetWindowSurface(window);
          buffer = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0,
                                        0, 0, 0);
        }
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
      printf("time since initialization %u\n", SDL_GetTicks());
      if ((SDL_GetTicks() - last_interrupt) > 1.0 / 60.0)
        {
          printf("interrupt?\n");
          if (cpu.interrupt_enabled)
            {
              printf("inside interrupt loop.\n");
              generate_interrupt(&cpu, 2);
              last_interrupt = SDL_GetTicks();
              update_graphics(&cpu, window, buffer);
            }
        }

      // 3 Update system state for display, input, and sound

      // Update the surface
      SDL_UpdateWindowSurface(window);

      // 4 Check for exit conditions
    }

  // Clean up resources and exit
  // Destroy window
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();
}

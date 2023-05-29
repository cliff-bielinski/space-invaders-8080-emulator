#include "emulator.h"
#include <ctype.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define JOYSTICK_DEAD_ZONE 8000

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Event e;
bool has_event = false;

static int speed = 1;
static bool should_quit = false;
static uint32_t curr_time = 0;
static uint32_t last_time = 0;
static uint32_t dt = 0;

void
io_loop(i8080 *cpu) // NOLINT(readability-function-cognitive-complexity)
{

  has_event = SDL_PollEvent(&e) != 0;
  if (has_event) // NOLINT
    {
      if (e.type == SDL_QUIT)
        {
          should_quit = true;
        }
      else if (e.type == SDL_KEYDOWN)
        {
          SDL_Scancode key = e.key.keysym.scancode;
          if (key == SDL_SCANCODE_C)
            {
              cpu->port1 |= 1 << 0; // NOLINT
            }
          else if (key == SDL_SCANCODE_2)
            {
              cpu->port1 |= 1 << 1; // NOLINT
            }
          else if (key == SDL_SCANCODE_RETURN)
            {
              cpu->port1 |= 1 << 2; // NOLINT
            }
          else if (key == SDL_SCANCODE_SPACE)
            {
              cpu->port1 |= 1 << 4; // NOLINT
              cpu->port2 |= 1 << 4; // NOLINT
            }
          else if (key == SDL_SCANCODE_LEFT)
            {
              cpu->port1 |= 1 << 5; // NOLINT
              cpu->port2 |= 1 << 5; // NOLINT
            }
          else if (key == SDL_SCANCODE_RIGHT)
            {
              cpu->port1 |= 1 << 6; // NOLINT
              cpu->port2 |= 1 << 6; // NOLINT
            }
          else if (key == SDL_SCANCODE_T)
            {
              cpu->port2 |= 1 << 2; // NOLINT
            }
          else if (key == SDL_SCANCODE_ESCAPE)
            {
              SDL_Event quit_event;
              quit_event.type = SDL_QUIT;
              SDL_PushEvent(&quit_event);
              //
              //
              //
              //
            }
          else if (key == SDL_SCANCODE_TAB)
            {
              speed = 5; // NOLINT
            }
        }
      else if (e.type == SDL_KEYUP)
        {
          SDL_Scancode key = e.key.keysym.scancode;
          if (key == SDL_SCANCODE_C)
            {
              cpu->port1 &= 0b11111110; // NOLINT
            }
          else if (key == SDL_SCANCODE_2)
            {
              cpu->port1 &= 0b11111101; // NOLINT
            }
          else if (key == SDL_SCANCODE_RETURN)
            {
              cpu->port1 &= 0b11111011; // NOLINT
            }
          else if (key == SDL_SCANCODE_SPACE)
            {
              cpu->port1 &= 0b11101111; // NOLINT
              cpu->port2 &= 0b11101111; // NOLINT
            }
          else if (key == SDL_SCANCODE_LEFT)
            {
              cpu->port1 &= 0b11011111; // NOLINT
              cpu->port2 &= 0b11011111; // NOLINT
            }
          else if (key == SDL_SCANCODE_RIGHT)
            {
              cpu->port1 &= 0b10111111; // NOLINT
              cpu->port2 &= 0b10111111; // NOLINT
            }
          else if (key == SDL_SCANCODE_T)
            {
              cpu->port2 &= 0b11111011; // NOLINT
            }
          else if (key == SDL_SCANCODE_TAB)
            {
              speed = 1;
            }
        }
      else if (e.type == SDL_JOYAXISMOTION)
        {
          if (e.jaxis.axis == 0) // NOLINT
            {
              if (e.jaxis.value < -JOYSTICK_DEAD_ZONE)
                {
                  cpu->port1 |= 1 << 5; // NOLINT
                  cpu->port2 |= 1 << 5; // NOLINT
                }
              else if (e.jaxis.value > JOYSTICK_DEAD_ZONE)
                {
                  cpu->port1 |= 1 << 6; // NOLINT
                  cpu->port2 |= 1 << 6; // NOLINT
                }
              else
                {
                  cpu->port1 &= 0b11011111; // NOLINT
                  cpu->port2 &= 0b11011111; // NOLINT

                  cpu->port1 &= 0b10111111; // NOLINT
                  cpu->port2 &= 0b10111111; // NOLINT
                }
            }
          else if (e.type == SDL_JOYBUTTONDOWN)
            {
              if (e.jbutton.button == 1) // NOLINT
                {
                  cpu->port1 |= 1 << 0; // NOLINT
                }
              else if (e.jbutton.button == 0) // NOLINT
                {
                  cpu->port1 |= 1 << 4; // NOLINT
                  cpu->port2 |= 1 << 4; // NOLINT
                }
              else if (e.jbutton.button == 8) // NOLINT
                {
                  cpu->port1 |= 1 << 2; // NOLINT
                }
              else if (e.jbutton.button == 9) // NOLINT
                {
                  cpu->port1 |= 1 << 1; // NOLINT
                }
              else if (e.jbutton.button == 13) // NOLINT
                {
                  cpu->port1 |= 1 << 5; // NOLINT
                  cpu->port2 |= 1 << 5; // NOLINT
                }
              else if (e.jbutton.button == 14) // NOLINT
                {
                  cpu->port1 |= 1 << 6; // NOLINT
                  cpu->port2 |= 1 << 6; // NOLINT
                }
              else if (e.jbutton.button == 4) // NOLINT
                {
                  break;
                }
            }
          else if (e.type == SDL_JOYBUTTONUP)
            {
              if (e.jbutton.button == 1) // NOLINT
                {
                  cpu->port1 &= 0b11111110; // NOLINT
                }
              else if (e.jbutton.button == 0) // NOLINT
                {
                  cpu->port1 &= 0b11101111; // NOLINT
                  cpu->port2 &= 0b11101111; // NOLINT
                }
              else if (e.jbutton.button == 8) // NOLINT
                {
                  cpu->port1 &= 0b11111011; // NOLINT
                }
              else if (e.jbutton.button == 9) // NOLINT
                {
                  cpu->port1 &= 0b11111101; // NOLINT
                }
              else if (e.jbutton.button == 13) // NOLINT
                {
                  cpu->port1 &= 0b11011111; // NOLINT
                  cpu->port2 &= 0b11011111; // NOLINT
                }
              else if (e.jbutton.button == 14) // NOLINT
                {
                  cpu->port1 &= 0b10111111; // NOLINT
                  cpu->port2 &= 0b10111111; // NOLINT
                }
            }
        }
    }

  // update
  has_event = false;
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

}

#define CLOCK_SPEED_MS 2000
#define TICK (1000 * (1.0 / 60.0))
#define CYCLES_PER_TICK (CLOCK_SPEED_MS * TICK)

int run_cpu(i8080 *cpu, int cycles);
int pflag = 0;
int dflag = 0;
SDL_Window *window = NULL;
SDL_Surface *screen_surface = NULL;
SDL_Surface *buffer = NULL;

int
main(int argc, char *argv[])
{
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
/*
  // initialize CPU state
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
    {
      SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
      return 1;
    }
  
  renderer = SDL_CreateRenderer(
    window, -1, SDL_RENDERER,ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
  SDL_LOG("unable to create renderer: %s", SDL_GetError());
  return 1;
  }

  SDL_Joystick *joystick = NULL;
  if (SDL_NumJoysticks() > 0)
    {
      joystick = SDL_JoystickOpen(0);
      if (joystick)
        {
          SDL_Log("Joystick successfully found");
        }
      else
        {
          SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failure opening joystick 0");
        }
    }
    */
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
  int num_cycles = CYCLES_PER_TICK / 2;

  // The surface contained by the window

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0)
    {
      fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n",
              SDL_GetError());
      exit(EXIT_FAILURE);
    }
  else
    {
      // Create window
      window = SDL_CreateWindow("Space Invaders Emulator",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
      if (window == NULL)
        {
          fprintf(stderr, "Window could not be created! SDL_Error: %s\n",
                  SDL_GetError());
          exit(EXIT_FAILURE);
        }
      else
        {
          // Get window surface
          screen_surface = SDL_GetWindowSurface(window);
          // NOLINTNEXTLINE
          buffer = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0,
                                        0, 0, 0);
        }
    }
  renderer = SDL_CreateRenderer(
    window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
  SDL_LOG("unable to create renderer: %s", SDL_GetError());
  return 1;
  }
  SDL_Joystick *joystick = NULL;
  if (SDL_NumJoysticks() > 0)
    {
      joystick = SDL_JoystickOpen(0);
      if (joystick)
        {
          SDL_Log("Joystick successfully found");
        }
      else
        {
          SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failure opening joystick 0");
        }
    }

  while (true)
    {
      if ((SDL_GetTicks() - last_tick) > TICK) // NOLINT
        {
          if (pflag)
            {
              printf("Current Tick: %d\n", SDL_GetTicks());
            }

          // run first half of tick cycles
          cycle_offset = run_cpu(&cpu, num_cycles - abs(cycle_offset));

          // first interrupt
          handle_interrupt(&cpu, 0x01);

          // run second half of tick cycles
          cycle_offset = run_cpu(&cpu, num_cycles - abs(cycle_offset));

          // second interrupt
          handle_interrupt(&cpu, 0x02);

          // set number of cycles for next tick
          num_cycles = CYCLES_PER_TICK / 2 - cycle_offset;

          // 3 Update system state for display, input, and sound
          // Update graphics after VBLANK int
          io_loop(&cpu);
          update_graphics(&cpu, buffer, screen_surface);
          SDL_UpdateWindowSurface(window);

          // 4 Check for exit conditions

          last_tick = SDL_GetTicks();
        }
    }

  // Destroy window
  SDL_DestroyWindow(window);
  // Quit SDL subsystems
  SDL_Quit();
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

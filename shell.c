#include "emulator.h"
#include <ctype.h>

//#include "SDL_nmix.h"
#include <unistd.h>
#define JOYSTICK_DEAD_ZONE 8000

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Event e;

static int speed = 1;
static bool should_quit = false;
static uint32_t curr_time = 0;
static uint32_t last_time = 0;
static uint32_t dt = 0;

void
io_loop(i8080 *cpu)
{
  curr_time = SDL_GetTicks();
  dt = curr_time - last_time;

  while (SDL_PollEvent(&e) != 0)
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
              cpu->port1 |= 1 << 0;
            }
          else if (key == SDL_SCANCODE_2)
            {
              cpu->port1 |= 1 << 1;
            }
          else if (key == SDL_SCANCODE_RETURN)
            {
              cpu->port1 |= 1 << 2;
            }
          else if (key == SDL_SCANCODE_SPACE)
            {
              cpu->port1 |= 1 << 4;
              cpu->port2 |= 1 << 4;
            }
          else if (key == SDL_SCANCODE_LEFT)
            {
              cpu->port1 |= 1 << 5;
              cpu->port2 |= 1 << 5;
            }
          else if (key == SDL_SCANCODE_RIGHT)
            {
              cpu->port1 |= 1 << 6;
              cpu->port2 |= 1 << 6;
            }
          else if (key == SDL_SCANCODE_T)
            {
              cpu->port2 |= 1 << 2;
            }
          else if (key == SDL_SCANCODE_ESCAPE)
            {
              break;
              //
              //
              //
              //
            }
          else if (key == SDL_SCANCODE_TAB)
            {
              speed = 5;
            }
        }
      else if (e.type == SDL_KEYUP)
        {
          SDL_Scancode key = e.key.keysym.scancode;
          if (key == SDL_SCANCODE_C)
            {
              cpu->port1 &= 0b11111110;
            }
          else if (key == SDL_SCANCODE_2)
            {
              cpu->port1 &= 0b11111101;
            }
          else if (key == SDL_SCANCODE_RETURN)
            {
              cpu->port1 &= 0b11111011;
            }
          else if (key == SDL_SCANCODE_SPACE)
            {
              cpu->port1 &= 0b11101111;
              cpu->port2 &= 0b11101111;
            }
          else if (key == SDL_SCANCODE_LEFT)
            {
              cpu->port1 &= 0b11011111;
              cpu->port2 &= 0b11011111;
            }
          else if (key == SDL_SCANCODE_RIGHT)
            {
              cpu->port1 &= 0b10111111;
              cpu->port2 &= 0b10111111;
            }
          else if (key == SDL_SCANCODE_T)
            {
              cpu->port2 &= 0b11111011;
            }
          else if (key == SDL_SCANCODE_TAB)
            {
              speed = 1;
            }
        }
      else if (e.type == SDL_JOYAXISMOTION)
        {
          if (e.jaxis.axis == 0)
            {
              if (e.jaxis.value < -JOYSTICK_DEAD_ZONE)
                {
                  cpu->port1 |= 1 << 5;
                  cpu->port2 |= 1 << 5;
                }
              else if (e.jaxis.value > JOYSTICK_DEAD_ZONE)
                {
                  cpu->port1 |= 1 << 6;
                  cpu->port2 |= 1 << 6;
                }
              else
                {
                  cpu->port1 &= 0b11011111;
                  cpu->port2 &= 0b11011111;

                  cpu->port1 &= 0b10111111;
                  cpu->port2 &= 0b10111111;
                }
            }
          else if (e.type == SDL_JOYBUTTONDOWN)
            {
              if (e.jbutton.button == 1)
                {
                  cpu->port1 |= 1 << 0;
                }
              else if (e.jbutton.button == 0)
                {
                  cpu->port1 |= 1 << 4;
                  cpu->port2 |= 1 << 4;
                }
              else if (e.jbutton.button == 8)
                {
                  cpu->port1 |= 1 << 2;
                }
              else if (e.jbutton.button == 9)
                {
                  cpu->port1 |= 1 << 1;
                }
              else if (e.jbutton.button == 13)
                {
                  cpu->port1 |= 1 << 5;
                  cpu->port2 |= 1 << 5;
                }
              else if (e.jbutton.button == 14)
                {
                  cpu->port1 |= 1 << 6;
                  cpu->port2 |= 1 << 6;
                }
              else if (e.jbutton.button == 4)
                {
                  break;
                }
            }
          else if (e.type == SDL_JOYBUTTONUP)
            {
              if (e.jbutton.button == 1)
                {
                  cpu->port1 &= 0b11111110;
                }
              else if (e.jbutton.button == 0)
                {
                  cpu->port1 &= 0b11101111;
                  cpu->port2 &= 0b11101111;
                }
              else if (e.jbutton.button == 8)
                {
                  cpu->port1 &= 0b11111011;
                }
              else if (e.jbutton.button == 9)
                {
                  cpu->port1 &= 0b11111101;
                }
              else if (e.jbutton.button == 13)
                {
                  cpu->port1 &= 0b11011111;
                  cpu->port2 &= 0b11011111;
                }
              else if (e.jbutton.button == 14)
                {
                  cpu->port1 &= 0b10111111;
                  cpu->port2 &= 0b10111111;
                }
            }
        }
    }

  // update
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  last_time = curr_time;
}

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
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
    {
      SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
      return 1;
    }
  /*
  renderer = SDL_CreateRenderer(
    window, -1, SDL_RENDERER,ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
  SDL_LOG("unable to create renderer: %s", SDL_GetError());
  return 1;
  }
*/
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

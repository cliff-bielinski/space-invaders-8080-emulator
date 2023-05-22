# compiler
CC = clang

# compiler flags
CFLAGS = -g -W -Wall -Wextra -pedantic `pkg-config --cflags --libs sdl2`

# linker flags LINKER_FLAGS = -lSDL2

# targets to build
TARGETS = disassembler_8080 shell

# build all non-testing executables
all: $(TARGETS)

# build disassembler executable
disassembler_8080:
	$(CC) $(CFLAGS) -o disassembler_8080 disassembler_8080.c

# build emulator object
emulator:
	$(CC) $(CFLAGS) -c emulator.c

# build shell executable
shell: emulator
	$(CC) $(CFLAGS) -c shell.c
	$(CC) $(CFLAGS) -o shell shell.o emulator.o

# build tests executable and run tests
test: emulator
	$(CC) $(CFLAGS) -c tests.c
	$(CC) $(CFLAGS) -o tests tests.o emulator.o -lcunit
	./tests

# removes existing objects and executables
clean:
	$(RM) *.o emulator tests shell disassembler_8080

# compiler
CC = clang

# compiler flags
CFLAGS = -g -W -Wall -Wextra -pedantic

# libraries to load
LIBS = 

# targets to build
TARGETS = disassembler_8080

# build all targets (default)
all: $(TARGETS)

$(TARGETS): $(TARGETS).c
	$(CC) $(CFLAGS) -o $(TARGETS) $(TARGETS).c $(LIBS)

# run tests
test:

# removes existing objects and executables
clean:
	$(RM) *.o $(TARGETS)
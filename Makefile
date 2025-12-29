CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

# Source files
SRC = src/transistor.c \
      src/gates.c \
      src/register.c \
      src/fulladder.c \
      src/decoderom.c \
      src/rcl.c \
      src/alu.c \
      src/regalu.c \
      src/pc.c \
      src/regfile.c \
      src/clock.c \
      src/timing.c \
      src/cpu.c

# Object directory
OBJDIR = .obj

# Convert src/foo.c -> .obj/src/foo.o
OBJ = $(SRC:%.c=$(OBJDIR)/%.o)

BIN = test/bin/test_cpu

all: $(BIN)

# Link
$(BIN): $(OBJ)
	@mkdir -p test/bin
	$(CC) $(OBJ) -o $(BIN)

# Compile rule (auto-create subdirectories)
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BIN)

.PHONY: all clean

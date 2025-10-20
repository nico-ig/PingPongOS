# Compiler and flags
CC = gcc
CFLAGS = -O0 -g
DFLAGS = -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L
TARGET = ppos

# Source and object files
SRCDIR = .
INCLUDES = -I$(SRCDIR) -I$(SRCDIR)/logger -I$(SRCDIR)/ppos_src -I$(SRCDIR)/timer -I$(SRCDIR)/queue -I$(SRCDIR)/dispatcher 
SOURCES = $(SRCDIR)/timer/timer.c $(SRCDIR)/queue/queue.c $(SRCDIR)/dispatcher/dispatcher.c $(SRCDIR)/ppos_src/ppos_core.c
OBJECTS = $(SOURCES:.c=.o)

# Test targets
TEST_SRCS = $(wildcard tests/*.c)
TEST_EXECS = $(patsubst tests/%.c,tests/bin/%,$(TEST_SRCS))

# Default target
all: $(TARGET)

# Main target
$(TARGET): $(OBJECTS) $(SRCDIR)/main.c
	$(CC) $(OBJECTS) $(SRCDIR)/main.o -o $(TARGET)

# Object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Debug build
debug: CFLAGS += $(DFLAGS) -DDEBUG
debug: purge $(TARGET)

# Log level builds
log_%: purge
	$(CC) $(CFLAGS) $(DFLAGS) -DLOG_LEVEL=$* $(INCLUDES) $(SOURCES) -o $(TARGET)

# Force rebuild
rebuild: purge all

# Build all test executables
tests: purge $(OBJECTS) $(TEST_EXECS)

# Create bin directory if it doesn't exist
tests/bin:
	mkdir -p tests/bin

# Build a single test executable
tests/bin/%: tests/%.c | tests/bin
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJECTS) $< -o $@

# Clean object files
clean:
	rm -f $(OBJECTS)

# Remove all generated files
purge: clean
	rm -f $(TARGET)
	rm -rf tests/bin

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  debug    - Build with debug flags"
	@echo "  log_N    - Build with log level N (e.g., log_1, log_2)"
	@echo "  tests     - Build all test executables"
	@echo "  clean    - Remove object files"
	@echo "  purge    - Remove all generated files"
	@echo "  rebuild  - Clean and rebuild"
	@echo "  help     - Show this help message"

.PHONY: all debug log_% clean purge rebuild help tests

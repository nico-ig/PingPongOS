# Compiler and flags
CC = gcc
CFLAGS = -O0 -g
DFLAGS = -std=c99 -Wall -Wextra
INCLUDES = -I./logger -I./ppos_core -I./dispatcher -I./queue

# Source files
SRCDIR = .
SOURCES = main.c ppos_core/ppos_core.c dispatcher/dispatcher.c queue/queue.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = ppos

# Default target
all: $(TARGET)

# Main target
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Debug build
debug: CFLAGS += $(DFLAGS) -DDEBUG
debug: purge $(TARGET)

# Log level builds
log_%: purge
	$(CC) $(CFLAGS) $(DFLAGS) -DLOG_LEVEL=$* $(INCLUDES) $(SOURCES) -o $(TARGET)

# Clean object files
clean:
	rm -f $(OBJECTS)

# Remove all generated files
purge: clean
	rm -f $(TARGET)

# Force rebuild
rebuild: purge all

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  debug    - Build with debug flags"
	@echo "  log_N    - Build with log level N (e.g., log_1, log_2)"
	@echo "  clean    - Remove object files"
	@echo "  purge    - Remove all generated files"
	@echo "  rebuild  - Clean and rebuild"
	@echo "  help     - Show this help message"

.PHONY: all debug log_% clean purge rebuild help

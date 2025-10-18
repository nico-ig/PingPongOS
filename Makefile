# Compiler and flags
CC = gcc
CFLAGS = -O0 -g
DFLAGS = -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L
TARGET = ppos

SRCDIR = .
INCLUDES = -I$(SRCDIR) -I$(SRCDIR)/logger -I$(SRCDIR)/ppos_src -I$(SRCDIR)/timer -I$(SRCDIR)/queue -I$(SRCDIR)/dispatcher 
SOURCES = $(SRCDIR)/timer/timer.c $(SRCDIR)/queue/queue.c $(SRCDIR)/dispatcher/dispatcher.c $(SRCDIR)/ppos_src/ppos_core.c $(SRCDIR)/main.c
OBJECTS = $(SOURCES:.c=.o)

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

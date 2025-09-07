# PingPongOS

## Project Organization

### File Structure

#### Core System Files
- **`ppos_core/`**
  - **`ppos_data.h`**
    - Global constants and macros  
    - System-wide type definitions
    - Task control block structure
    - Queue node structure  
    - Error code enumerations

  - **`ppos.h`** *(read-only interface)*
    - Public API function prototypes
    - Task creation/management interfaces  
    - System initialization declarations

  - **`ppos_core.c`**
    - Core OS kernel public API implementation
    - Task lifecycle management public functions
    - Context switching public interface
    - System initialization public routines

  - **`ppos_core_internal.h`** - Core system internal functions
  - **`ppos_core_internal.c`** - Core system internal implementations
    - Internal system initialization functions
    - Internal task management utilities
    - Stack management and cleanup functions
    - Task ID assignment and core utilities

#### Subsystems
- **`dispatcher/`**
  - **`dispatcher.c`**  
    - Main dispatcher loop
    - Task scheduling coordination
  - **`dispatcher.h`** - Dispatcher API
  - **`dispatcher_internal.h`** - Internal dispatcher functions
  - **`dispatcher_internal.c`** - Internal dispatcher implementations
    - Task selection and scheduling

- **`queue/`**  
  - **`queue.c`**
    - FIFO queue implementation
    - Queue manipulation primitives  
    - Queue traversal helpers
  - **`queue.h`** - Queue public API
  - **`queue_internal.h`** - Internal queue functions and types
  - **`queue_internal.c`** - Internal queue function implementations

- **`task/`**
  - **`task.h`** - Task management public API
    - Task creation and initialization functions
    - Task execution and lifecycle management
    - Task status handling and cleanup
  - **`task.c`** - Task management public implementations
    - Public API functions (no underscore prefix)
    - Error handling with LOG_ERR for API failures
  - **`task_internal.h`** - Task management internal functions
  - **`task_internal.c`** - Task management internal implementations
    - Internal helper functions (underscore prefix)
    - Stack management and context creation
    - Internal task lifecycle operations

#### Support Components
- **`logger/`** - System logging facility
    - **`logger.h`** - Logging API
    
- **`main.c`** - Bootstrap and test entry point

## Coding Standards

### Function Naming and Organization
- No function should have `<name>(void)` as parameter, should be just `name()`
- Functions that start with `_` should always be static and not appear in headers
- Functions that start with `_` should be on internal files only
- Public API functions should NOT start with underscore prefix
- Internal functions should have descriptive suffixes (e.g., `_internal`) for clarity

### Logging Standards
- Functions that start with `_` shouldn't have LOG_INFO logs
- Internal functions shouldn't have LOG_ERR, should use LOG_WARN instead
- The API function should log with LOG_ERR if the internal function returns an error
- Log messages of errors that aren't handled should be logged with LOG_WARN
- Every function should log with TRACE when entered and exiting, with return value or (void)
- Every if that calls goto exit should have a LOG_WARN or LOG_ERR
- Utils files like queue shouldn't have LOG_INFO logs
- There should be at most one LOG_INFO log per function
- Every API function should have a LOG_INFO log (and only one)

### Code Structure and Architecture
- Functions should have only one point of return
- Functions should always return a value (id, pointer, or status enum)
- Every case of a switch should call a function instead of having many lines (unless it's just a log)
- Every declaration should be on a header file, with comments with brief, parameters and return
- There shouldn't be any global variables
- There shouldn't be comments in the code, with exception of descriptions in headers

### File Organization
- When creating a new file, add it to the README.md
- Every header should have a ifndef guard that starts with two underscores
- Separate public API from internal implementation across different files
- Use consistent directory structure with clear separation of concerns

## TODO List

### Build System
- [ ] Use cmake to generate makefile

### Code Quality & Architecture

### Technical Improvements
- [ ] Fix valgrind errors

### Style & Formatting

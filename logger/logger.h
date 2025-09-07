#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Log colors
#define LOG_RED     "\033[31m"
#define LOG_YELLOW  "\033[33m"
#define LOG_BLUE    "\033[34m"
#define LOG_CYAN    "\033[36m"
#define LOG_GREEN   "\033[32m"
#define LOG_RESET   "\033[0m"

// Log level definitions
#define LOG_LEVEL_ERR   0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_TRACE 4

#if defined(DEBUG)
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

// Define log macros
#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_LEVEL_ERR)
#define LOG_ERR(fmt, ...) fprintf(stderr, LOG_RED "[ERR] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#define LOG_ERR0(fmt) fprintf(stderr, LOG_RED "[ERR] " fmt LOG_RESET "\n")
#else
#define LOG_ERR(fmt, ...) ((void)0)
#define LOG_ERR0(fmt) ((void)0)
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_LEVEL_WARN)
#define LOG_WARN(fmt, ...) printf(LOG_YELLOW "[WARN] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#define LOG_WARN0(fmt) printf(LOG_YELLOW "[WARN] " fmt LOG_RESET "\n")
#else
#define LOG_WARN(fmt, ...) ((void)0)
#define LOG_WARN0(fmt) ((void)0)
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_LEVEL_INFO)
#define LOG_INFO(fmt, ...) printf(LOG_GREEN "[INFO] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#define LOG_INFO0(fmt) printf(LOG_GREEN "[INFO] " fmt LOG_RESET "\n")
#else
#define LOG_INFO(fmt, ...) ((void)0)
#define LOG_INFO0(fmt) ((void)0)
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_LEVEL_DEBUG)
#define LOG_DEBUG(fmt, ...) printf(LOG_BLUE "[DEBUG] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#define LOG_DEBUG0(fmt) printf(LOG_BLUE "[DEBUG] " fmt LOG_RESET "\n")
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#define LOG_DEBUG0(fmt) ((void)0)
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_LEVEL_TRACE)
#define LOG_TRACE(fmt, ...) printf(LOG_CYAN "[TRACE] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#define LOG_TRACE0(fmt) printf(LOG_CYAN "[TRACE] " fmt LOG_RESET "\n")
#else
#define LOG_TRACE(fmt, ...) ((void)0)
#define LOG_TRACE0(fmt) ((void)0)
#endif

#endif
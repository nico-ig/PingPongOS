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

// Default log level
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

// Define log macros
#if LOG_LEVEL >= LOG_LEVEL_ERR
#define LOG_ERR(fmt, ...)   printf(LOG_RED "[ERR] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#else
#define LOG_ERR(fmt, ...)   ;
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN(fmt, ...)  printf(LOG_YELLOW "[WARN] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...)  ;
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...)  printf(LOG_GREEN "[INFO] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)  ;
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) printf(LOG_BLUE "[DEBUG] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ;
#endif

#if LOG_LEVEL >= LOG_LEVEL_TRACE
#define LOG_TRACE(fmt, ...) printf(LOG_CYAN "[TRACE] " fmt LOG_RESET "\n", ##__VA_ARGS__)
#else
#define LOG_TRACE(fmt, ...) ;
#endif

#endif
/*
  This file is part of NightShift.

  NightShift is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NightShift is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NightShift. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TRACE_H
#define TRACE_H

#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdbool.h>

typedef enum {
  LOG_DEBUG = 0,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_GUARD
} log_levels;

extern bool __debug_mode;

#define UNKNOWN_FILENAME \
  "<UNKNOWN FILENAME>"

#define __FILENAME__ \
  ({ \
    char *__file_name = NULL; \
    __file_name = basename(__FILE__); \
    if (__file_name == NULL) { \
      __file_name = (char *)UNKNOWN_FILENAME; \
    } \
    __file_name; \
  })

#define DEBUG_TRACE(message, ...) \
  do { \
    trace_log(LOG_DEBUG, message, ##__VA_ARGS__); \
  } while(0);

#define INFO_TRACE(message, ...) \
  do { \
    trace_log(LOG_INFO, message, ##__VA_ARGS__); \
  } while(0);

#define WARNING_TRACE(message, ...) \
  do { \
    trace_log(LOG_WARNING, message, ##__VA_ARGS__); \
  } while(0);

#define ERROR_TRACE(message, ...) \
  do { \
    trace_log(LOG_ERROR, message, ##__VA_ARGS__); \
  } while(0);

#define TRAP(condition, message, ...) \
  do { \
    if (condition) { \
      int last_errno = errno; \
      DEBUG_TRACE("Bug has been trapped by condition '%s' at %s:%d.\n", \
        #condition, __FILENAME__, __LINE__); \
      ERROR_TRACE(message, ##__VA_ARGS__); \
      INFO_TRACE("Last system error code description: %s\n", \
        trace_strerror(last_errno)); \
      if (__debug_mode) { \
        abort(); \
      } else { \
        exit(EXIT_FAILURE); \
      } \
    } \
  } while(0);

#define BUG(message, ...) \
  do { \
    DEBUG_TRACE("Bug has been trapped at %s:%d.\n", \
      __FILENAME__, __LINE__); \
    ERROR_TRACE(message, ##__VA_ARGS__); \
    if (__debug_mode) { \
      abort(); \
    } else { \
      exit(EXIT_FAILURE); \
    } \
  } while(0);

void trace_log(log_levels level, const char * format, ...);
void set_debug_traces(bool enabled);
char *trace_strerror(int error_code);
#endif

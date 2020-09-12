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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include "trace.h"

#define trace_log_step(stream, message) \
  do { \
    (void)pthread_mutex_lock(&trace_log_guard); \
    fprintf(stream, "%s", message); \
    fflush(stream); \
    (void)pthread_mutex_unlock(&trace_log_guard); \
  } while(0);

#define TRACE_BAD_LEVEL \
  "ERROR: bad log level was detected\n"
#define TRACE_CANNOT_MAP_LEVEL \
  "ERROR: log cannot map level to message\n"
#define TRACE_CANNOT_MAKE_MESSAGE \
  "ERROR: log cannot prepare final message\n"
#define TRACE_BAD_ERRNO \
  "ERROR: bad errno code was provided"
#define TRACE_TOO_SHORT_ERROR_MESSAGE \
  "ERROR: error message has too few lenght to map errno description"

static const char *trace_log_prefixes[] = {
  [LOG_DEBUG] = "DEBUG: ",
  [LOG_INFO] = "INFO: ",
  [LOG_WARNING] = "WARNING: ",
  [LOG_ERROR] = "ERROR: "
};

bool __debug_mode = false;
static pthread_mutex_t trace_log_guard = PTHREAD_MUTEX_INITIALIZER;

static char *trace_level_to_prefix(log_levels log_level)
{
  if ((log_level < LOG_DEBUG) || (log_level >= LOG_GUARD)) {
    trace_log_step(stderr, TRACE_BAD_LEVEL);
    return (char *)trace_log_prefixes[LOG_ERROR];
  }
  return (char *)trace_log_prefixes[log_level];
}

void set_debug_traces(bool enabled)
{
  __debug_mode = enabled;
}

void trace_log(log_levels level, const char *format, ...)
{
  char message[PATH_MAX]; // let it be PATH_MAX so far
  ssize_t cursor = 0;
  va_list args;

  if ((level == LOG_DEBUG) && !__debug_mode) {
    return;
  }

  (void)memset((void *)message, 0, PATH_MAX);

  cursor = snprintf(message, PATH_MAX, "%s", trace_level_to_prefix(level));
  if ((cursor < 0) || (cursor > PATH_MAX)) {
    trace_log_step(stderr, TRACE_CANNOT_MAP_LEVEL);
    return;
  }

  va_start(args, format);
  cursor = vsnprintf(message + cursor, PATH_MAX - cursor - 1, format, args);
  va_end(args);
  if ((cursor < 0) || (cursor > PATH_MAX - cursor - 1)) {
    trace_log_step(stderr, TRACE_CANNOT_MAKE_MESSAGE);
    return;
  }

  message[PATH_MAX - 1] = '\0';

  trace_log_step((level == LOG_ERROR) ? stderr : stdout, message);
}

char *trace_strerror(int error_code)
{
  static __thread char error_message[PATH_MAX]; // let it be PATH_MAX so far
  int iret = -1;

  (void)memset((void *)error_message, 0, PATH_MAX);
  iret = strerror_r(error_code, error_message, PATH_MAX);

  if (iret == -1) {
    switch (errno) {
      case EINVAL:
        (void)memcpy(error_message, TRACE_BAD_ERRNO, sizeof(TRACE_BAD_ERRNO));
        break;
      case ERANGE:
        (void)memcpy(error_message, TRACE_TOO_SHORT_ERROR_MESSAGE, sizeof(TRACE_TOO_SHORT_ERROR_MESSAGE));
        break;
    }
  }

  return error_message;
}

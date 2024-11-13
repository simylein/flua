#include "config.h"
#include <stdarg.h>
#include <stdio.h>

const char *purple = "\x1b[35m";
const char *blue = "\x1b[34m";
const char *cyan = "\x1b[36m";
const char *green = "\x1b[32m";
const char *yellow = "\x1b[33m";
const char *red = "\x1b[31m";
const char *bold = "\x1b[1m";
const char *normal = "\x1b[22m";
const char *reset = "\x1b[39m";

void req(const char *message, ...) {
  if (log_requests >= 1) {
    fprintf(stdout, "%s%sflua %sreq%s ", bold, blue, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void res(const char *message, ...) {
  if (log_responses >= 1) {
    fprintf(stdout, "%s%sflua %sres%s ", bold, blue, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void trace(const char *message, ...) {
  if (log_level >= 6) {
    fprintf(stdout, "%s%sflua %strace%s%s ", bold, blue, blue, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void debug(const char *message, ...) {
  if (log_level >= 5) {
    fprintf(stdout, "%s%sflua %sdebug%s%s ", bold, blue, cyan, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void info(const char *message, ...) {
  if (log_level >= 4) {
    fprintf(stdout, "%s%sflua %sinfo%s%s ", bold, blue, green, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void warn(const char *message, ...) {
  if (log_level >= 3) {
    fprintf(stderr, "%s%sflua %swarn%s%s ", bold, blue, yellow, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
  }
}

void error(const char *message, ...) {
  if (log_level >= 2) {
    fprintf(stderr, "%s%sflua %serror%s%s ", bold, blue, red, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
  }
}

void fatal(const char *message, ...) {
  if (log_level >= 1) {
    fprintf(stderr, "%s%sflua %sfatal%s%s ", bold, blue, purple, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
  }
}

#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

const char *purple = "\x1b[35m";
const char *blue = "\x1b[34m";
const char *cyan = "\x1b[36m";
const char *green = "\x1b[32m";
const char *yellow = "\x1b[33m";
const char *red = "\x1b[31m";
const char *bold = "\x1b[1m";
const char *normal = "\x1b[22m";
const char *reset = "\x1b[39m";

char *timestamp(void) {
  static char buffer[9] = "??:??:??\0";
  time_t now = time(NULL);
  int elapsed = now % 86400;
  int seconds = elapsed % 60;
  int minutes = elapsed / 60 % 60;
  int hours = elapsed / 3600;
  buffer[0] = (char)hours / 10 + 48;
  buffer[1] = (char)hours % 10 + 48;
  buffer[3] = (char)minutes / 10 + 48;
  buffer[4] = (char)minutes % 10 + 48;
  buffer[6] = (char)seconds / 10 + 48;
  buffer[7] = (char)seconds % 10 + 48;
  return buffer;
}

void req(const char *message, ...) {
  if (log_requests >= 1) {
    fprintf(stdout, "%s%sflua%s%s %s %sreq%s ", bold, blue, reset, normal, timestamp(), bold, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void res(const char *message, ...) {
  if (log_responses >= 1) {
    fprintf(stdout, "%s%sflua%s%s %s %sres%s ", bold, blue, reset, normal, timestamp(), bold, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void trace(const char *message, ...) {
  if (log_level >= 6) {
    fprintf(stdout, "%s%sflua%s%s %s %s%strace%s%s ", bold, blue, reset, normal, timestamp(), bold, blue, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void debug(const char *message, ...) {
  if (log_level >= 5) {
    fprintf(stdout, "%s%sflua%s%s %s %s%sdebug%s%s ", bold, blue, reset, normal, timestamp(), bold, cyan, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void info(const char *message, ...) {
  if (log_level >= 4) {
    fprintf(stdout, "%s%sflua%s%s %s %s%sinfo%s%s ", bold, blue, reset, normal, timestamp(), bold, green, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
  }
}

void warn(const char *message, ...) {
  if (log_level >= 3) {
    fprintf(stderr, "%s%sflua%s%s %s %s%swarn%s%s ", bold, blue, reset, normal, timestamp(), bold, yellow, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
  }
}

void error(const char *message, ...) {
  if (log_level >= 2) {
    fprintf(stderr, "%s%sflua%s%s %s %s%serror%s%s ", bold, blue, reset, normal, timestamp(), bold, red, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
  }
}

void fatal(const char *message, ...) {
  if (log_level >= 1) {
    fprintf(stderr, "%s%sflua%s%s %s %s%sfatal%s%s ", bold, blue, reset, normal, timestamp(), bold, purple, reset, normal);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
  }
}

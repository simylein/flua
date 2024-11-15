#include <stdio.h>

char *human_duration(long nanoseconds) {
  static char duration_buffer[8];
  if (nanoseconds < 1000) {
    snprintf(duration_buffer, sizeof(duration_buffer), "%ldns", nanoseconds);
  } else if (nanoseconds < 1000000) {
    snprintf(duration_buffer, sizeof(duration_buffer), "%ldus", nanoseconds / 1000);
  } else if (nanoseconds < 1000000000) {
    snprintf(duration_buffer, sizeof(duration_buffer), "%ldms", nanoseconds / 1000000);
  } else {
    snprintf(duration_buffer, sizeof(duration_buffer), "%lds", nanoseconds / 1000000000);
  }
  return duration_buffer;
}

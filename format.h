#include <time.h>

void human_time(char (*buffer)[8], time_t seconds);
void human_duration(char (*buffer)[8], struct timespec *start, struct timespec *stop);
void human_bytes(char (*buffer)[8], size_t bytes);

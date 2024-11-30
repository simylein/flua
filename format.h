#include <time.h>

void human_duration(char (*buffer)[8], struct timespec *start, struct timespec *stop);
void human_bytes(char (*buffer)[8], size_t bytes);
int bin_to_hex(char *buffer, const size_t buffer_len, const void *bin, const size_t bin_len);
int hex_to_bin(void *buffer, const size_t buffer_len, const char *hex, const size_t hex_len);

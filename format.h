#include <time.h>

void human_duration(char (*buffer)[8], struct timespec *start, struct timespec *stop);
void human_bytes(char (*buffer)[8], size_t bytes);
int bin_to_hex(char *buffer, const size_t buffer_size, const unsigned char *bin, const int bin_size);
int hex_to_bin(unsigned char *buffer, const size_t buffer_size, const char *hex, const size_t hex_size);

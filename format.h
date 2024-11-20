#include <time.h>

void human_duration(char (*buffer)[8], struct timespec *start, struct timespec *stop);
void human_bytes(char (*buffer)[8], size_t bytes);
int human_uuid(char (*buffer)[33], const unsigned char *binary_uuid, const int binary_uuid_size);
int binary_uuid(unsigned char (*buffer)[16], const char *hex_uuid, const size_t hex_uuid_size);

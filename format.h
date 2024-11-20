#include <time.h>

char *human_duration(struct timespec *start, struct timespec *stop);
char *human_bytes(size_t bytes);
int human_uuid(char (*buffer)[33], const unsigned char *binary_uuid, const int binary_uuid_size);
int binary_uuid(unsigned char (*buffer)[16], const char *hex_uuid, const size_t hex_uuid_size);

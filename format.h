#include <time.h>

char *human_duration(struct timespec *start, struct timespec *stop);
char *human_bytes(size_t bytes);
const char *human_uuid(const unsigned char *binary_uuid, const int binary_uuid_size);
const unsigned char *binary_uuid(const char *uuid, const size_t uuid_size);

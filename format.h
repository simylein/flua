#include <time.h>

char *human_duration(struct timespec *start, struct timespec *stop);
char *human_bytes(size_t bytes);
char *human_uuid(const unsigned char *id, const int id_size);

#include "response.h"
#include <stdint.h>
#include <time.h>

typedef struct file_t {
	int fd;
	char *ptr;
	size_t len;
	time_t age;
} file_t;

void file(const char *file_path, file_t *file, response_t *response);

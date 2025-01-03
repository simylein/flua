#include "response.h"
#include <stdint.h>

typedef struct file_t {
	char *ptr;
	size_t len;
} file_t;

void file(const char *file_path, file_t *file, response_t *response);

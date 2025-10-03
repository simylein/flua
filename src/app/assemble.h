#pragma once

#include "file.h"
#include <stdint.h>

typedef struct inject_t {
	char *ptr;
	uint8_t len;
} inject_t;

int assemble(file_t *asset);

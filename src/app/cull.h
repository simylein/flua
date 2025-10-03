#pragma once

#include "file.h"
#include <stdint.h>

typedef struct cull_t {
	char *start;
	uint8_t start_ind;
	uint8_t start_len;
	char *end;
	uint8_t end_ind;
	uint8_t end_len;
} cull_t;

uint8_t cull_flight(file_t *file);

#pragma once

#include "file.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct class_t {
	char *ptr;
	uint8_t len;
	bool known;
} class_t;

typedef struct breakpoint_t {
	const char *tag;
	const uint8_t tag_len;
	const char *prefix;
	const uint8_t prefix_len;
	const char *suffix;
	const uint8_t suffix_len;
} breakpoint_t;

int extract(file_t *file, class_t (*classes)[256], uint16_t *classes_len);

int hydrate(file_t *file, class_t (*classes)[256], uint16_t *classes_len);

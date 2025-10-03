#pragma once

#include "strn.h"
#include <stdint.h>
#include <stdio.h>

typedef struct request_t request_t;

typedef struct response_t {
	uint16_t status;
	strn8_t head;
	strn16_t header;
	strn32_t body;
} response_t;

void response_init(response_t *response, char *buffer);
size_t response(request_t *req, response_t *res, char *buffer);

void header_write(response_t *response, const char *format, ...) __attribute__((format(printf, 2, 3)));
void body_write(response_t *response, const void *buffer, size_t buffer_len);

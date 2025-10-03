#pragma once

#include "strn.h"
#include <stdint.h>
#include <stdio.h>

typedef struct response_t response_t;

typedef struct request_t {
	strn8_t method;
	strn8_t pathname;
	strn16_t search;
	strn8_t protocol;
	strn16_t header;
	strn32_t body;
} request_t;

void request_init(request_t *request);
void request(char *buffer, size_t length, request_t *req, response_t *res);

const char *param_find(request_t *request, uint8_t offset, uint8_t *length);
const char *header_find(request_t *request, const char *key);
const char *body_read(request_t *request, uint32_t length);

#include "response.h"
#include <stdint.h>

#pragma once
typedef struct request_t {
	char *method;
	uint8_t method_len;
	char *pathname;
	uint8_t pathname_len;
	char *search;
	uint16_t search_len;
	char *protocol;
	uint8_t protocol_len;
	char *header;
	uint16_t header_len;
	char *body;
	size_t body_len;
} request_t;

void request(char *buffer, size_t length, request_t *req, response_t *res);

const char *find_header(request_t *request, const char *key);

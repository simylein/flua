#include <stdint.h>
#include <stdio.h>

typedef struct request_t request_t;

#pragma once
typedef struct response_t {
	uint16_t status;
	size_t head_len;
	char (*header)[2048];
	size_t header_len;
	char (*body)[96128];
	size_t body_len;
} response_t;

size_t response(char *buffer, request_t *req, response_t *res);

void append_header(response_t *response, const char *format, ...) __attribute__((format(printf, 2, 3)));
void append_body(response_t *response, const void *buffer, size_t buffer_len);

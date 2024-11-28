#include <stdio.h>

#pragma once
typedef struct response_t {
	int status;
	size_t head_len;
	char header[3584];
	size_t header_len;
	char body[61440];
	size_t body_len;
} response_t;

size_t response(char *buffer, response_t *res);

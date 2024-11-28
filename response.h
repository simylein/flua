#include <stdio.h>

#pragma once
typedef struct Response {
	int status;
	size_t head_len;
	char header[1536];
	size_t header_len;
	char body[22528];
	size_t body_len;
} Response;

size_t response(char *buffer, Response *res);

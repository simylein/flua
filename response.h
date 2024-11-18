#include <stdio.h>

#pragma once
typedef struct Response {
	int status;
	char header[1024];
	char body[6144];
	size_t body_len;
} Response;

size_t response(char (*buffer)[8192], Response *res);

#include <stdio.h>

#pragma once
typedef struct Response {
	int status;
	size_t head_len;
	char header[2048];
	size_t header_len;
	char body[8192];
	size_t body_len;
} Response;

size_t response(char (*buffer)[12288], Response *res);

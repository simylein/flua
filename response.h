#include <stdio.h>

#pragma once
typedef struct Response {
	int status;
	char header[2048];
	char body[8192];
	size_t body_len;
} Response;

size_t response(char (*buffer)[12288], Response *res);

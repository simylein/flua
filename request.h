#include "response.h"
#include <arpa/inet.h>

#pragma once
typedef struct Request {
	char method[16];
	char pathname[128];
	char search[256];
	char protocol[16];
	char header[2048];
	char body[8192];
	size_t body_len;
} Request;

void request(char (*buffer)[12288], ssize_t length, Request *req, Response *res);

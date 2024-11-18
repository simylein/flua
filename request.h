#include "response.h"
#include <arpa/inet.h>

#pragma once
typedef struct Request {
	char method[8];
	char pathname[64];
	char search[128];
	char protocol[16];
	char header[1024];
	char body[6144];
	size_t body_len;
} Request;

void request(char (*buffer)[8192], ssize_t length, Request *req, Response *res);

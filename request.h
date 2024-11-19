#include "response.h"
#include <arpa/inet.h>

#pragma once
typedef struct Request {
	char method[16];
	size_t method_len;
	char pathname[128];
	size_t pathname_len;
	char search[256];
	size_t search_len;
	char protocol[16];
	size_t protocol_len;
	char header[2048];
	size_t header_len;
	char body[8192];
	size_t body_len;
} Request;

void request(char (*buffer)[12288], ssize_t length, Request *req, Response *res);

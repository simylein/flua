#include "response.h"
#include <arpa/inet.h>

#pragma once
typedef struct request_t {
	char method[16];
	size_t method_len;
	char pathname[128];
	size_t pathname_len;
	char search[256];
	size_t search_len;
	char protocol[16];
	size_t protocol_len;
	char header[3584];
	size_t header_len;
	char body[61440];
	size_t body_len;
} request_t;

void request(char *buffer, ssize_t length, request_t *req, response_t *res);

#include "response.h"
#include <arpa/inet.h>

#pragma once
typedef struct request_t {
	char *method;
	size_t method_len;
	char *pathname;
	size_t pathname_len;
	char *search;
	size_t search_len;
	char *protocol;
	size_t protocol_len;
	char *header;
	size_t header_len;
	char *body;
	size_t body_len;
} request_t;

void request(char *buffer, ssize_t length, request_t *req, response_t *res);

const char *find_header(request_t *request, const char *key);

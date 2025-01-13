#include "request.h"
#include "response.h"
#include "utils.h"
#include <arpa/inet.h>
#include <string.h>

void request(char *buffer, ssize_t length, request_t *req, response_t *res) {
	int stage = 0;
	size_t index = 0;

	const size_t method_index = index;
	while (stage == 0 && req->method_len < 8 && index < (size_t)length) {
		char *byte = &buffer[index];
		if (*byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == ' ') {
			stage = 1;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			req->method_len++;
		}
		index++;
	}
	req->method = &buffer[method_index];
	if (stage == 0 || req->method_len == 0) {
		res->status = 501;
		return;
	}

	const size_t pathname_index = index;
	while (stage == 1 && req->pathname_len < 128 && index < (size_t)length) {
		char *byte = &buffer[index];
		if (*byte == '?') {
			stage = 2;
		} else if (*byte == ' ') {
			stage = 3;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			req->pathname_len++;
		}
		index++;
	}
	req->pathname = &buffer[pathname_index];
	if (stage == 1 || req->pathname_len == 0) {
		res->status = 414;
		return;
	}

	const size_t search_index = index;
	while (stage == 2 && req->search_len < 256 && index < (size_t)length) {
		char *byte = &buffer[index];
		if (*byte == ' ') {
			stage = 3;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			req->search_len++;
		}
		index++;
	}
	req->search = &buffer[search_index];
	if (stage == 2) {
		res->status = 414;
		return;
	}

	const size_t protocol_index = index;
	while ((stage == 3 || stage == 4) && req->protocol_len < 16 && index < (size_t)length) {
		char *byte = &buffer[index];
		if (*byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == '\r') {
			stage = 4;
		} else if (*byte == '\n') {
			stage = 5;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			req->protocol_len++;
		}
		index++;
	}
	req->protocol = &buffer[protocol_index];
	if (stage == 3 || req->protocol_len == 0) {
		res->status = 505;
		return;
	}
	if (stage == 4 || req->protocol_len == 0) {
		res->status = 400;
		return;
	}

	int header_key = 1;
	const size_t header_index = index;
	while ((stage >= 5 && stage <= 8) && req->header_len < 2048 && index < (size_t)length) {
		char *byte = &buffer[index];
		if (header_key && *byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == ':') {
			header_key = 0;
		}
		if (*byte == '\r' || *byte == '\n') {
			header_key = 1;
			stage += 1;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			stage = 5;
		}
		req->header_len++;
		index++;
	}
	req->header = &buffer[header_index];
	if (stage >= 5 && stage <= 7) {
		res->status = 431;
		return;
	}
	if (stage == 8) {
		res->status = 400;
		return;
	}

	req->body_len = (size_t)length - index;
	req->body = &buffer[index];
}

const char *find_header(request_t *request, const char *key) {
	const char *header = strncasestrn(request->header, request->header_len, key, strlen(key));
	if (header) {
		header += strlen(key);
		if (header[0] == ' ') {
			header += 1;
		}
		return header;
	}
	return NULL;
}

#include "request.h"
#include "config.h"
#include "response.h"
#include "strn.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void request_init(request_t *request) {
	uint32_t offset = 0;

	request->method.len = 0;
	request->method.cap = 8;
	offset += request->method.cap;

	request->pathname.len = 0;
	request->pathname.cap = 128;
	offset += request->pathname.cap;

	request->search.len = 0;
	request->search.cap = 256;
	offset += request->search.cap;

	request->protocol.len = 0;
	request->protocol.cap = 16;
	offset += request->protocol.cap;

	request->header.len = 0;
	request->header.cap = 2048;
	offset += request->header.cap;

	request->body.len = 0;
	request->body.cap = receive_buffer - offset;
	offset += request->body.cap;
}

void request(char *buffer, size_t length, request_t *req, response_t *res) {
	uint8_t stage = 0;
	size_t index = 0;

	const size_t method_index = index;
	while (stage == 0 && req->method.len < req->method.cap && index < length) {
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
			req->method.len++;
		}
		index++;
	}
	req->method.ptr = &buffer[method_index];
	if (stage == 0 || req->method.len == 0) {
		res->status = 501;
		return;
	}

	const size_t pathname_index = index;
	while (stage == 1 && req->pathname.len < req->pathname.cap && index < length) {
		char *byte = &buffer[index];
		if (*byte == '?') {
			stage = 2;
		} else if (*byte == ' ') {
			stage = 3;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			req->pathname.len++;
		}
		index++;
	}
	req->pathname.ptr = &buffer[pathname_index];
	if (stage == 1 || req->pathname.len == 0) {
		res->status = 414;
		return;
	}

	const size_t search_index = index;
	while (stage == 2 && req->search.len < req->search.cap && index < length) {
		char *byte = &buffer[index];
		if (*byte == ' ') {
			stage = 3;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			req->search.len++;
		}
		index++;
	}
	req->search.ptr = &buffer[search_index];
	if (stage == 2) {
		res->status = 414;
		return;
	}

	const size_t protocol_index = index;
	while ((stage == 3 || stage == 4) && req->protocol.len < req->protocol.cap && index < length) {
		char *byte = &buffer[index];
		if (*byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == '\r' || *byte == '\n') {
			stage += 1;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			req->protocol.len++;
		}
		index++;
	}
	req->protocol.ptr = &buffer[protocol_index];
	if (stage == 3 || req->protocol.len == 0) {
		res->status = 505;
		return;
	}
	if (stage == 4 || req->protocol.len == 0) {
		res->status = 400;
		return;
	}

	bool header_key = true;
	const size_t header_index = index;
	while ((stage >= 3 && stage <= 6) && req->header.len < req->header.cap && index < length) {
		char *byte = &buffer[index];
		if (header_key == true && *byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == ':') {
			header_key = false;
		}
		if (*byte == '\r' || *byte == '\n') {
			header_key = true;
			stage += 1;
		} else if (*byte <= '\037') {
			res->status = 400;
			return;
		} else {
			stage = 3;
		}
		req->header.len++;
		index++;
	}
	req->header.ptr = &buffer[header_index];
	if (stage >= 3 && stage <= 5) {
		res->status = 431;
		return;
	}
	if (stage == 6) {
		res->status = 400;
		return;
	}

	req->body.len = (uint32_t)(length - index);
	req->body.ptr = &buffer[index];
}

const char *param_find(request_t *request, uint8_t offset, uint8_t *length) {
	if (request->pathname.len < offset) {
		return NULL;
	}

	while (offset + *length < request->pathname.len) {
		if (request->pathname.ptr[offset + *length] == '/') {
			break;
		}
		*length += 1;
	}

	return &request->pathname.ptr[offset];
}

const char *header_find(request_t *request, const char *key) {
	const char *header = strncasestrn(request->header.ptr, request->header.len, key, strlen(key));
	if (header == NULL) {
		return NULL;
	}

	header += strlen(key);
	if (header[0] == ':') {
		header += 1;
	}
	if (header[0] == ' ') {
		header += 1;
	}

	return header;
}

const char *body_read(request_t *request, uint32_t length) {
	char *ptr = &(request->body.ptr[request->body.pos]);
	request->body.pos += length;
	return ptr;
}

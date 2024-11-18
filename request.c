#include "request.h"
#include <arpa/inet.h>
#include <string.h>

void request(char (*buffer)[8192], ssize_t length, Request *req, Response *res) {
	int stage = 0;
	size_t index = 0;

	size_t method_length = 0;
	const size_t method_index = index;
	while (stage == 0 && method_length < sizeof(req->method) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == ' ') {
			stage = 1;
		} else if (*byte >= '\0' && *byte <= '\037') {
			res->status = 400;
			return;
		} else {
			method_length++;
		}
		index++;
	}
	memcpy(req->method, &(*buffer)[method_index], method_length);
	req->method[method_length] = '\0';
	if (stage == 0) {
		res->status = 501;
		return;
	}

	size_t pathname_length = 0;
	const size_t pathname_index = index;
	while (stage == 1 && pathname_length < sizeof(req->pathname) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte == '?') {
			stage = 2;
		} else if (*byte == ' ') {
			stage = 3;
		} else if (*byte >= '\0' && *byte <= '\037') {
			res->status = 400;
			return;
		} else {
			pathname_length++;
		}
		index++;
	}
	memcpy(req->pathname, &(*buffer)[pathname_index], pathname_length);
	req->pathname[pathname_length] = '\0';
	if (stage == 1) {
		res->status = 414;
		return;
	}

	size_t search_length = 0;
	const size_t search_index = index;
	while (stage == 2 && search_length < sizeof(req->search) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte == ' ') {
			stage = 3;
		} else if (*byte >= '\0' && *byte <= '\037') {
			res->status = 400;
			return;
		} else {
			search_length++;
		}
		index++;
	}
	memcpy(req->search, &(*buffer)[search_index], search_length);
	req->search[search_length] = '\0';
	if (stage == 2) {
		res->status = 414;
		return;
	}

	size_t protocol_length = 0;
	const size_t protocol_index = index;
	while ((stage == 3 || stage == 4) && protocol_length < sizeof(req->protocol) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == '\r') {
			stage = 4;
		} else if (*byte == '\n') {
			stage = 5;
		} else if (*byte >= '\0' && *byte <= '\037') {
			res->status = 400;
			return;
		} else {
			protocol_length++;
		}
		index++;
	}
	memcpy(req->protocol, &(*buffer)[protocol_index], protocol_length);
	req->protocol[protocol_length] = '\0';
	if (stage == 3) {
		res->status = 505;
		return;
	}
	if (stage == 4) {
		res->status = 400;
		return;
	}

	int header_key = 1;
	size_t header_length = 0;
	const size_t header_index = index;
	while ((stage >= 5 && stage <= 8) && header_length < sizeof(req->header) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (header_key && *byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == ':') {
			header_key = 0;
		}
		if (*byte == '\r' || *byte == '\n') {
			header_key = 1;
			stage += 1;
		} else if (*byte >= '\0' && *byte <= '\037') {
			res->status = 400;
			return;
		} else {
			stage = 5;
		}
		header_length++;
		index++;
	}
	memcpy(req->header, &(*buffer)[header_index], header_length);
	req->header[header_length] = '\0';
	if (stage >= 5 && stage <= 7) {
		res->status = 431;
		return;
	}
	if (stage == 8) {
		res->status = 400;
		return;
	}

	size_t body_length = (size_t)length - index;
	if (body_length > sizeof(req->body)) {
		res->status = 413;
		return;
	}

	memcpy(req->body, &(*buffer)[index], body_length);
	req->body_len = body_length;
}

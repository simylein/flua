#include "request.h"
#include <arpa/inet.h>
#include <string.h>

Request request(char (*buffer)[8192], ssize_t length) {
	struct Request req = {
			.method = {0}, .pathname = {0}, .search = {0}, .protocol = {0}, .header = {0}, .body = {0}, .status = 0};

	int stage = 0;
	size_t index = 0;

	size_t method_length = 0;
	const size_t method_index = index;
	while (stage == 0 && method_length < sizeof(req.method) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == ' ') {
			stage = 1;
		} else if (*byte >= '\0' && *byte <= '\037') {
			req.status = 400;
			return req;
		} else {
			method_length++;
		}
		index++;
	}
	memcpy(req.method, &(*buffer)[method_index], method_length);
	req.method[method_length] = '\0';
	if (stage == 0) {
		req.status = 501;
		return req;
	}

	size_t pathname_length = 0;
	const size_t pathname_index = index;
	while (stage == 1 && pathname_length < sizeof(req.pathname) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte == '?') {
			stage = 2;
		} else if (*byte == ' ') {
			stage = 3;
		} else if (*byte >= '\0' && *byte <= '\037') {
			req.status = 400;
			return req;
		} else {
			pathname_length++;
		}
		index++;
	}
	memcpy(req.pathname, &(*buffer)[pathname_index], pathname_length);
	req.pathname[pathname_length] = '\0';
	if (stage == 1) {
		req.status = 414;
		return req;
	}

	size_t search_length = 0;
	const size_t search_index = index;
	while (stage == 2 && search_length < sizeof(req.search) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte == ' ') {
			stage = 3;
		} else if (*byte >= '\0' && *byte <= '\037') {
			req.status = 400;
			return req;
		} else {
			search_length++;
		}
		index++;
	}
	memcpy(req.search, &(*buffer)[search_index], search_length);
	req.search[search_length] = '\0';
	if (stage == 2) {
		req.status = 414;
		return req;
	}

	size_t protocol_length = 0;
	const size_t protocol_index = index;
	while ((stage == 3 || stage == 4) && protocol_length < sizeof(req.protocol) - 1 && index < (size_t)length) {
		char *byte = &(*buffer)[index];
		if (*byte >= 'A' && *byte <= 'Z') {
			*byte += 32;
		}
		if (*byte == '\r') {
			stage = 4;
		} else if (*byte == '\n') {
			stage = 5;
		} else if (*byte >= '\0' && *byte <= '\037') {
			req.status = 400;
			return req;
		} else {
			protocol_length++;
		}
		index++;
	}
	memcpy(req.protocol, &(*buffer)[protocol_index], protocol_length);
	req.protocol[protocol_length] = '\0';
	if (stage == 3) {
		req.status = 505;
		return req;
	}
	if (stage == 4) {
		req.status = 400;
		return req;
	}

	int header_key = 1;
	size_t header_length = 0;
	const size_t header_index = index;
	while ((stage >= 5 && stage <= 8) && header_length < sizeof(req.header) - 1 && index < (size_t)length) {
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
			req.status = 400;
			return req;
		} else {
			stage = 5;
		}
		header_length++;
		index++;
	}
	memcpy(req.header, &(*buffer)[header_index], header_length);
	req.header[header_length] = '\0';
	if (stage >= 5 && stage <= 7) {
		req.status = 431;
		return req;
	}
	if (stage == 8) {
		req.status = 400;
		return req;
	}

	size_t body_length = (size_t)length - index;
	if (body_length > sizeof(req.body) - 1) {
		req.status = 413;
		return req;
	}

	memcpy(req.body, &(*buffer)[index], body_length);
	req.body[body_length] = '\0';

	return req;
}

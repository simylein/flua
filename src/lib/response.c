#include "response.h"
#include "config.h"
#include "request.h"
#include "status.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void response_init(response_t *response, char *buffer) {
	uint32_t offset = 0;

	response->status = 0;

	response->head.ptr = &buffer[offset];
	response->head.len = 0;
	response->head.cap = 128;
	offset += response->head.cap;

	response->header.ptr = &buffer[offset];
	response->header.len = 0;
	response->header.cap = 2048;
	offset += response->header.cap;

	response->body.ptr = &buffer[offset];
	response->body.len = 0;
	response->body.cap = send_buffer - offset;
	offset += response->body.cap;
}

size_t response(request_t *req, response_t *res, char *buffer) {
	res->head.len += (uint8_t)sprintf(res->head.ptr, "HTTP/1.1 %hu %s\r\n", res->status, status_text(res->status));
	if (res->header.len > 0) {
		memmove(&buffer[res->head.len], res->header.ptr, res->header.len);
	}
	memcpy(&buffer[res->head.len + res->header.len], "\r\n", 2);
	res->header.len += 2;
	if (req->method.len == 4 && memcmp(req->method.ptr, "head", req->method.len) == 0) {
		res->body.len = 0;
	}
	return res->head.len + res->header.len + res->body.len;
}

void header_write(response_t *response, const char *format, ...) {
	va_list args;
	va_start(args, format);
	response->header.len += (uint16_t)vsprintf(response->header.ptr + response->header.len, format, args);
	va_end(args);
}

void body_write(response_t *response, const void *buffer, size_t buffer_len) {
	memcpy(response->body.ptr + response->body.len, buffer, buffer_len);
	response->body.len += (uint32_t)buffer_len;
}

#include "response.h"
#include "status.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

size_t response(char *buffer, response_t *res) {
	size_t bytes = 0;
	bytes += (size_t)sprintf(buffer + bytes, "HTTP/1.1 %d %s\r\n", res->status, status_text(res->status));
	res->head_len += bytes;
	if (res->header_len > 0) {
		memcpy(buffer + bytes, res->header, res->header_len);
		bytes += res->header_len;
	}
	memcpy(buffer + bytes, "\r\n", 2);
	bytes += 2;
	if (res->body_len > 0) {
		memcpy(buffer + bytes, res->body, res->body_len);
		bytes += res->body_len;
	}
	return bytes;
}

void append_header(response_t *response, const char *format, ...) {
	va_list args;
	va_start(args, format);
	response->header_len += (size_t)vsprintf(&response->header[response->header_len], format, args);
	va_end(args);
}

void append_body(response_t *response, const void *buffer, size_t buffer_len) {
	memcpy(&response->body[response->body_len], buffer, buffer_len);
	response->body_len += buffer_len;
}

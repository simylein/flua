#include "response.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

size_t response(char (*buffer)[20480], Response *res) {
	size_t bytes = 0;
	bytes += (size_t)sprintf(*buffer + bytes, "HTTP/1.1 %d %s\r\n", res->status, status_text(res->status));
	res->head_len += bytes;
	if (res->header_len > 0) {
		memcpy(*buffer + bytes, res->header, res->header_len);
		bytes += res->header_len;
	}
	memcpy(*buffer + bytes, "\r\n", 2);
	bytes += 2;
	if (res->body_len > 0) {
		memcpy(*buffer + bytes, res->body, res->body_len);
		bytes += res->body_len;
	}
	return bytes;
}

#include "response.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

size_t response(char (*buffer)[8192], Response *res) {
	size_t bytes = 0;
	bytes += (size_t)sprintf(*buffer + bytes, "HTTP/1.1 %d %s\r\n", res->status, status_text(res->status));
	size_t header_len = strlen(res->header);
	if (header_len > 0) {
		memcpy(*buffer + bytes, res->header, header_len);
		bytes += header_len;
	} else {
		char *end = "\r\n";
		size_t end_len = strlen(end);
		memcpy(*buffer + bytes, end, end_len);
		bytes += end_len;
	}
	if (res->body_len > 0) {
		memcpy(*buffer + bytes, res->body, res->body_len);
		bytes += res->body_len;
	}
	return bytes;
}

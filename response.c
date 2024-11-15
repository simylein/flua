#include "response.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

int response(char (*buffer)[8192], Response *res) {
	int bytes = 0;
	bytes += snprintf(*buffer + bytes, (int)sizeof(*buffer) - bytes, "HTTP/1.1 %d %s\r\n", res->status, status_text(res->status));
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
	size_t body_len = strlen(res->body);
	if (body_len > 0) {
		memcpy(*buffer + bytes, res->body, body_len);
		bytes += body_len;
	}
	return bytes;
}

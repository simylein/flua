#include "request.h"
#include "response.h"
#include <string.h>

void null_init(request_t *request, response_t *response) {
	request->method[0] = '\0';
	request->method_len = 0;
	request->pathname[0] = '\0';
	request->pathname_len = 0;
	request->search[0] = '\0';
	request->search_len = 0;
	request->protocol[0] = '\0';
	request->protocol_len = 0;
	request->header[0] = '\0';
	request->header_len = 0;
	request->body_len = 0;

	response->status = 0;
	response->head_len = 0;
	response->header[0] = '\0';
	response->header_len = 0;
	response->body_len = 0;
}

char *strcasestr(const char *buffer, const char *buf) {
	size_t buffer_len = strlen(buffer);
	size_t buf_len = strlen(buf);

	size_t index = 0;
	size_t ind = 0;

	while (index < buffer_len) {
		if (buffer[index] == buf[ind] || (buffer[index] >= 'A' && buffer[index] <= 'Z' && buffer[index] + 32 == buf[ind])) {
			ind++;
			if (ind == buf_len) {
				return (char *)&buffer[index + 1 - buf_len];
			}
		} else {
			ind = 0;
		}
		index++;
	}

	return NULL;
}

char *strncasestrn(const char *buffer, size_t buffer_len, const char *buf, size_t buf_len) {
	size_t index = 0;
	size_t ind = 0;

	while (index < buffer_len) {
		if (buffer[index] == buf[ind] || (buffer[index] >= 'A' && buffer[index] <= 'Z' && buffer[index] + 32 == buf[ind])) {
			ind++;
			if (ind == buf_len) {
				return (char *)&buffer[index + 1 - buf_len];
			}
		} else {
			ind = 0;
		}
		index++;
	}

	return NULL;
}

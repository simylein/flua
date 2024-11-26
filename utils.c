#include "request.h"
#include "response.h"

void null_init(struct Request *req, struct Response *res) {
	req->method[0] = '\0';
	req->method_len = 0;
	req->pathname[0] = '\0';
	req->pathname_len = 0;
	req->search[0] = '\0';
	req->search_len = 0;
	req->protocol[0] = '\0';
	req->protocol_len = 0;
	req->header[0] = '\0';
	req->header_len = 0;
	req->body_len = 0;

	res->status = 0;
	res->head_len = 0;
	res->header[0] = '\0';
	res->header_len = 0;
	res->body_len = 0;
}

const char *strncasestr(const char *buffer, size_t buffer_len, const char *buf, size_t buf_len) {
	size_t ind = 0;
	size_t index = 0;

	while (index < buffer_len) {
		if (buffer[index] == buf[ind] || (buffer[index] >= 'A' && buffer[index] <= 'Z' && buffer[index] + 32 == buf[ind])) {
			ind++;
			if (ind == buf_len) {
				return &buffer[index + 1 - buf_len];
			}
		} else {
			ind = 0;
		}
		index++;
	}

	return NULL;
}

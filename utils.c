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

int strnfind(char *buffer, const size_t buffer_len, const char *prefix, const char *suffix, char **string, size_t *string_len,
						 size_t string_max_len) {
	size_t prefix_ind = 0;
	size_t prefix_len = strlen(prefix);

	size_t suffix_ind = 0;
	size_t suffix_len = strlen(suffix);

	size_t index = 0;

	while (prefix_ind < prefix_len && index < buffer_len) {
		if (buffer[index] == prefix[prefix_ind]) {
			prefix_ind++;
		} else {
			prefix_ind = 0;
		}
		index++;
	}
	if (prefix_ind != prefix_len) {
		return -1;
	}

	*string = &buffer[index];
	*string_len = 0;

	while ((suffix_len == 0 || suffix_ind < suffix_len) && index < buffer_len) {
		if (buffer[index] == suffix[suffix_ind]) {
			suffix_ind++;
		} else {
			if (*string_len + 1 > string_max_len) {
				break;
			}
			(*string_len)++;
		}
		index++;
	}
	if (suffix_ind != suffix_len) {
		return -1;
	}

	return 0;
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

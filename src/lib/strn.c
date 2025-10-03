#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int strnfind(const char *buffer, const size_t buffer_len, const char *prefix, const char *suffix, const char **string,
						 size_t *string_len, size_t string_len_max) {
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
			if (*string_len + 1 > string_len_max) {
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

const char *strncasestrn(const char *buffer, size_t buffer_len, const char *buf, size_t buf_len) {
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

int strnto8(const char *string, const size_t string_len, uint8_t *value) {
	for (size_t index = 0; index < string_len; index++) {
		if (string[index] < '0' || string[index] > '9') {
			return -1;
		}

		uint8_t digit = (uint8_t)string[index] - '0';
		*value = (uint8_t)(*value * 10 + digit);
	}

	return 0;
}

int strnto16(const char *string, const size_t string_len, uint16_t *value) {
	for (size_t index = 0; index < string_len; index++) {
		if (string[index] < '0' || string[index] > '9') {
			return -1;
		}

		uint16_t digit = (uint16_t)string[index] - '0';
		*value = (uint16_t)(*value * 10 + digit);
	}

	return 0;
}

int strnto32(const char *string, const size_t string_len, uint32_t *value) {
	for (size_t index = 0; index < string_len; index++) {
		if (string[index] < '0' || string[index] > '9') {
			return -1;
		}

		uint32_t digit = (uint32_t)string[index] - '0';
		*value = (uint32_t)(*value * 10 + digit);
	}

	return 0;
}

int strnto64(const char *string, const size_t string_len, uint64_t *value) {
	for (size_t index = 0; index < string_len; index++) {
		if (string[index] < '0' || string[index] > '9') {
			return -1;
		}

		uint64_t digit = (uint64_t)string[index] - '0';
		*value = (uint64_t)(*value * 10 + digit);
	}

	return 0;
}

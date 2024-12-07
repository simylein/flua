#include <stdint.h>
#include <stdio.h>

const char charset[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
												 "abcdefghijklmnopqrstuvwxyz"
												 "0123456789&$";

const uint8_t lookup[256] = {
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 63,	 255, 62,	 255, 255, 255, 255, 255, 255, 255, 255, 255,
		52,	 53,	54,	 55,	56,	 57,	58,	 59,	60,	 61,	255, 255, 255, 255, 255, 255, 255, 0,		1,	 2,		3,	 4,		5,	 6,
		7,	 8,		9,	 10,	11,	 12,	13,	 14,	15,	 16,	17,	 18,	19,	 20,	21,	 22,	23,	 24,	25,	 255, 255, 255, 255, 255,
		255, 26,	27,	 28,	29,	 30,	31,	 32,	33,	 34,	35,	 36,	37,	 38,	39,	 40,	41,	 42,	43,	 44,	45,	 46,	47,	 48,
		49,	 50,	51,	 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};

int base64_encode(char *buf, const size_t buf_len, const void *bin, const size_t bin_len) {
	if (buf_len < bin_len / 3 * 4 + 3) {
		return -1;
	}
	const uint8_t *binary = (const uint8_t *)bin;
	size_t buffer_index = 0;
	size_t binary_index = 0;
	while (binary_index < bin_len) {
		uint8_t alpha = binary_index < bin_len ? binary[binary_index++] : 0;
		uint8_t bravo = binary_index < bin_len ? binary[binary_index++] : 0;
		uint8_t charlie = binary_index < bin_len ? binary[binary_index++] : 0;
		uint32_t tri = ((uint32_t)alpha << 16) | ((uint32_t)bravo << 8) | (uint32_t)charlie;
		buf[buffer_index++] = charset[(tri >> 18) & 0x3f];
		buf[buffer_index++] = charset[(tri >> 12) & 0x3f];
		buf[buffer_index++] = binary_index - 1 < bin_len ? charset[(tri >> 6) & 0x3f] : '=';
		buf[buffer_index++] = binary_index < bin_len ? charset[tri & 0x3f] : '=';
	}
	buf[buffer_index] = '\0';
	return 0;
}

int base64_decode(void *bin, const size_t bin_len, const char *buf, const size_t buf_len) {
	if (buf_len - 3 < bin_len / 3 * 4) {
		return -1;
	}
	const uint8_t *buffer = (uint8_t *)buf;
	uint8_t *binary = (uint8_t *)bin;
	size_t buffer_index = 0;
	size_t binary_index = 0;
	while (buffer_index < buf_len) {
		uint8_t alpha = buffer_index < buf_len ? lookup[buffer[buffer_index++]] : 0;
		uint8_t bravo = buffer_index < buf_len ? lookup[buffer[buffer_index++]] : 0;
		uint8_t charlie = buffer_index < buf_len ? lookup[buffer[buffer_index++]] : 0;
		uint8_t delta = buffer_index < buf_len ? lookup[buffer[buffer_index++]] : 0;
		if (alpha == 0xff || bravo == 0xff || (charlie == 0xff && buf[buffer_index - 2] != '=') ||
				(delta == 0xff && buf[buffer_index - 1] != '=')) {
			return -1;
		}
		uint32_t quad = ((uint32_t)alpha << 18) | ((uint32_t)bravo << 12) | ((uint32_t)charlie << 6) | (uint32_t)delta;
		binary[binary_index++] = (quad >> 16) & 0xff;
		if (binary_index < bin_len && buf[buffer_index - 2] != '=') {
			binary[binary_index++] = (quad >> 8) & 0xff;
		}
		if (binary_index < bin_len && buf[buffer_index - 1] != '=') {
			binary[binary_index++] = quad & 0xff;
		}
	}
	return 0;
}

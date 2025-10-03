#include <stdint.h>

uint8_t significant_bytes(uint64_t value) {
	uint8_t bytes = 0;
	for (uint8_t index = 0; index < 8; index++) {
		if ((value >> (index * 8)) & 0xff) {
			bytes = index + 1;
		}
	}
	return bytes;
}

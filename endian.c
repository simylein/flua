#include <arpa/inet.h>
#include <stdint.h>

#ifndef htonll
uint64_t htonll(uint64_t value) {
	const int test = 42;
	if (*(const char *)&test == 42) {
		uint32_t high_bits = htonl((uint32_t)(value >> 32));
		uint32_t low_bits = htonl((uint32_t)(value & 0xffffffff));
		return (((uint64_t)low_bits) << 32) | high_bits;
	} else {
		return value;
	}
}
#endif

#ifndef ntohll
uint64_t ntohll(uint64_t value) {
	const int test = 42;
	if (*(const char *)&test == 42) {
		uint32_t high_bits = ntohl((uint32_t)(value >> 32));
		uint32_t low_bits = ntohl((uint32_t)(value & 0xffffffff));
		return (((uint64_t)low_bits) << 32) | high_bits;
	} else {
		return value;
	}
}
#endif

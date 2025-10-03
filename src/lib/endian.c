#include <arpa/inet.h>
#include <stdint.h>

uint16_t hton16(uint16_t value) { return htons(value); }

uint16_t ntoh16(uint16_t value) { return ntohs(value); }

uint32_t hton32(uint32_t value) { return htonl(value); }

uint32_t ntoh32(uint32_t value) { return ntohl(value); }

uint64_t hton64(uint64_t value) {
	uint32_t upper = htonl((uint32_t)(value >> 32));
	uint32_t lower = htonl((uint32_t)(value & 0xffffffff));
	return (((uint64_t)lower) << 32) | upper;
}

uint64_t ntoh64(uint64_t value) {
	uint32_t upper = ntohl((uint32_t)(value >> 32));
	uint32_t lower = ntohl((uint32_t)(value & 0xffffffff));
	return (((uint64_t)lower) << 32) | upper;
}

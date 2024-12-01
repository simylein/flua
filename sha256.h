#include <stdint.h>

void sha256(const void *data, size_t data_len, uint8_t (*hash)[32]);

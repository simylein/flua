#pragma once

#include <stdint.h>
#include <stdlib.h>

void sha256(const void *data, size_t data_len, uint8_t (*hash)[32]);
void sha256_hmac(const uint8_t *key, const size_t key_len, const void *data, const size_t data_len, uint8_t (*hmac)[32]);

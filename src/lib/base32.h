#pragma once

#include <stdio.h>

int base32_encode(char *buf, const size_t buf_len, const void *bin, const size_t bin_len);
int base32_decode(void *bin, const size_t bin_len, const char *buf, const size_t buf_len);

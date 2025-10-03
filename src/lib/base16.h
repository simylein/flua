#pragma once

#include <stdio.h>

int base16_encode(char *buf, const size_t buf_len, const void *bin, const size_t bin_len);
int base16_decode(void *bin, const size_t bin_len, const char *buf, const size_t buf_len);

#pragma once

#include <stdint.h>
#include <stdio.h>

typedef struct strn8_t {
	char *ptr;
	uint8_t len;
	uint8_t cap;
	uint8_t pos;
} strn8_t;

typedef struct strn16_t {
	char *ptr;
	uint16_t len;
	uint16_t cap;
	uint16_t pos;
} strn16_t;

typedef struct strn32_t {
	char *ptr;
	uint32_t len;
	uint32_t cap;
	uint32_t pos;
} strn32_t;

int strnfind(const char *buffer, const size_t buffer_len, const char *prefix, const char *suffix, const char **string,
						 size_t *string_len, size_t string_len_max);

const char *strncasestrn(const char *buffer, size_t buffer_len, const char *buf, size_t buf_len);

int strnto8(const char *string, const size_t string_len, uint8_t *value);
int strnto16(const char *string, const size_t string_len, uint16_t *value);
int strnto32(const char *string, const size_t string_len, uint32_t *value);
int strnto64(const char *string, const size_t string_len, uint64_t *value);

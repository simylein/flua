#include "request.h"
#include <stdint.h>
#include <time.h>

#pragma once
typedef struct bwt_t {
	uint8_t id[16];
	time_t iat;
	time_t exp;
} bwt_t;

int sign_bwt(char (*buffer)[88], const uint8_t *id, const size_t id_len);
int verify_bwt(const char *cookie, const size_t cookie_len, bwt_t *bwt);

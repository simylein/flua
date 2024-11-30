#include "request.h"
#include <time.h>

#pragma once
typedef struct bwt_t {
	unsigned char id[16];
	time_t iat;
	time_t exp;
} bwt_t;

int sign_bwt(char (*buffer)[65], const unsigned char *id, const size_t id_len);
int verify_bwt(const char *cookie, bwt_t *bwt);

#pragma once

#include "../lib/bwt.h"
#include "user.h"
#include <sqlite3.h>
#include <stdint.h>

typedef struct friend_t {
	uint8_t id[16];
	uint8_t user_id[16];
} friend_t;

uint16_t find_friend_by_user_id(sqlite3 *database, bwt_t *bwt_t, user_t *user, friend_t *friend);

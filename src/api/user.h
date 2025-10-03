#pragma once

#include "../lib/bwt.h"
#include "../lib/response.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>

enum visibility_t {
	private = 0,
	friends = 1,
	public = 2,
};

typedef struct user_t {
	uint8_t id[16];
	char username[17];
	uint8_t visibility;
} user_t;

uint16_t find_user_by_id(sqlite3 *database, uint8_t (*user_id)[16], user_t *user);
uint16_t find_user_by_name(sqlite3 *database, const char *name, size_t name_len, user_t *user);

void find_user(sqlite3 *database, bwt_t *bwt, response_t *response);
void update_user(sqlite3 *database, bwt_t *bwt, user_t *user, response_t *response);
void delete_user(sqlite3 *database, bwt_t *bwt, response_t *response);

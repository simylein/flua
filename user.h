#include "bwt.h"
#include "response.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct user_t {
	uint8_t id[16];
	char username[17];
	bool public;
} user_t;

uint16_t find_user_by_id(sqlite3 *database, uint8_t (*user_id)[16], user_t *user);
uint16_t find_user_by_name(sqlite3 *database, char *name, size_t name_len, user_t *user);

void find_user(sqlite3 *database, bwt_t *bwt, response_t *response);
void delete_user(sqlite3 *database, bwt_t *bwt, response_t *response);

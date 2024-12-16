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

int find_user_by_id(sqlite3 *database, uint8_t (*user_id)[16], user_t *user);
int find_user_by_name(sqlite3 *database, char *name, size_t name_len, user_t *user);

void delete_user(sqlite3 *database, bwt_t *bwt, response_t *response);

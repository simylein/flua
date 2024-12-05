#include "response.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct user_t {
	uint8_t id[16];
	char username[17];
	bool public;
} user_t;

int find_user_by_name(sqlite3 *database, char *name, user_t *user);

#include "response.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct user_t {
	uint8_t id[16];
	char username[17];
	bool public;
} user_t;

bool find_user(sqlite3 *database, char *name, user_t *user, response_t *response);

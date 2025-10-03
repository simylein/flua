#pragma once

#include "../lib/request.h"
#include "../lib/response.h"
#include <sqlite3.h>
#include <stdint.h>

void create_signin(sqlite3 *database, char *username, uint8_t username_len, char *password, uint8_t password_len,
									 response_t *response);
void create_signup(sqlite3 *database, char *username, uint8_t username_len, char *password, uint8_t password_len,
									 response_t *response);

#include "../lib/bwt.h"
#include "../lib/config.h"
#include "../lib/logger.h"
#include "../lib/request.h"
#include "../lib/response.h"
#include "../lib/sha256.h"
#include "user.h"
#include <sqlite3.h>
#include <stdint.h>
#include <string.h>

void create_signin(sqlite3 *database, char *username, uint8_t username_len, char *password, uint8_t password_len,
									 response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "select id from user where username = ? and password = ?";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	uint8_t hash[32];
	sha256(password, password_len, &hash);

	sqlite3_bind_text(stmt, 1, username, username_len, SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 2, hash, sizeof(hash), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		if (id_len != sizeof(((user_t *)0)->id)) {
			error("id length %zu does not match buffer length %zu\n", id_len, sizeof(((user_t *)0)->id));
			response->status = 500;
			goto cleanup;
		}
		char bwt[109];
		uint8_t data[4];
		if (bwt_sign(&bwt, (uint8_t (*)[16])id, &data) == -1) {
			response->status = 500;
			goto cleanup;
		}
		header_write(response, "set-cookie:auth=%.*s;Path=/;Max-Age=%u;SameSite=Strict;HttpOnly;\r\n", (int)sizeof(bwt), bwt,
								 bwt_ttl);
	} else if (result == SQLITE_DONE) {
		warn("invalid password for %.*s\n", (int)username_len, username);
		response->status = 401;
		goto cleanup;
	} else {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	info("user %.*s signed in\n", (int)username_len, username);
	response->status = 201;

cleanup:
	sqlite3_finalize(stmt);
}

void create_signup(sqlite3 *database, char *username, uint8_t username_len, char *password, uint8_t password_len,
									 response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "insert into user (id, username, password, visibility) values (randomblob(16), ?, ?, ?) returning id";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	uint8_t hash[32];
	sha256(password, password_len, &hash);

	sqlite3_bind_text(stmt, 1, username, username_len, SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 2, hash, sizeof(hash), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, private);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		if (id_len != sizeof(((user_t *)0)->id)) {
			error("id length %zu does not match buffer length %zu\n", id_len, sizeof(((user_t *)0)->id));
			response->status = 500;
			goto cleanup;
		}
		char bwt[109];
		uint8_t data[4];
		if (bwt_sign(&bwt, (uint8_t (*)[16])id, &data) == -1) {
			response->status = 500;
			goto cleanup;
		}
		header_write(response, "set-cookie:auth=%.*s;Path=/;Max-Age=%u;SameSite=Strict;HttpOnly;\r\n", (int)sizeof(bwt), bwt,
								 bwt_ttl);
	} else if (result == SQLITE_CONSTRAINT) {
		warn("username %.*s already taken\n", (int)username_len, username);
		response->status = 409;
		goto cleanup;
	} else {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	info("user %.*s signed up\n", (int)username_len, username);
	response->status = 201;

cleanup:
	sqlite3_finalize(stmt);
}

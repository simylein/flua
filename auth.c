#include "bwt.h"
#include "config.h"
#include "format.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "sha256.h"
#include <sqlite3.h>
#include <stdint.h>
#include <string.h>

void create_signin(sqlite3 *database, char *username, char *password, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "select id from user where username = ? and password = ?";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	uint8_t hash[32];
	sha256(password, strlen(password), &hash);

	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 2, hash, sizeof(hash), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		char bwt[89];
		if (sign_bwt(&bwt, id, id_len) == -1) {
			response->status = 500;
			goto cleanup;
		}
		append_header(response, "set-cookie:auth=%s;Path=/;Max-Age=%d;SameSite=Strict;HttpOnly;\r\n", bwt, bwt_ttl);
	} else if (result == SQLITE_DONE) {
		warn("invalid password for %s\n", username);
		response->status = 401;
		goto cleanup;
	} else {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to execute statement\n");
		response->status = 500;
		goto cleanup;
	}

	info("user %s signed in\n", username);
	response->status = 201;

cleanup:
	sqlite3_finalize(stmt);
}

void create_signup(sqlite3 *database, char *username, char *password, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "insert into user (id, username, password, public) values (randomblob(16), ?, ?, 0) returning id";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	uint8_t hash[32];
	sha256(password, strlen(password), &hash);

	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 2, hash, sizeof(hash), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		char bwt[89];
		if (sign_bwt(&bwt, id, id_len) == -1) {
			response->status = 500;
			goto cleanup;
		}
		append_header(response, "set-cookie:auth=%s;Path=/;Max-Age=%d;SameSite=Strict;HttpOnly;\r\n", bwt, bwt_ttl);
	} else if (result == SQLITE_CONSTRAINT) {
		warn("username %s already taken\n", username);
		response->status = 409;
		goto cleanup;
	} else {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to execute statement\n");
		response->status = 500;
		goto cleanup;
	}

	info("user %s signed up\n", username);
	response->status = 201;

cleanup:
	sqlite3_finalize(stmt);
}

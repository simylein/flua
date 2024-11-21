#include "database.h"
#include "format.h"
#include "logger.h"
#include "response.h"
#include <sqlite3.h>
#include <string.h>

void user_signin(char *username, char *password, Response *response) {
	info("user %s signing in\n", username);

	sqlite3_stmt *stmt;

	const char *sql = "select id from user where username = ? and password = ? returning id";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const unsigned char *id = sqlite3_column_blob(stmt, 0);
		const int id_size = sqlite3_column_bytes(stmt, 0);
		char uuid[33];
		if (human_uuid(&uuid, id, id_size) == -1) {
			error("failed to convert uuid to hex\n");
			response->status = 500;
			goto cleanup;
		}
		// TODO: sign a jwt with id in the payload bay
		response->header_len =
				(size_t)sprintf(response->header, "set-cookie:bearer=%s;Path=/;Max-Age=7200;HttpOnly;\r\n\r\n", uuid);
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

cleanup:
	sqlite3_finalize(stmt);
}

void user_signup(char *username, char *password, Response *response) {
	info("user %s signing up\n", username);

	sqlite3_stmt *stmt;

	const char *sql = "insert into user (id, username, password) values (randomblob(16), ?, ?) returning id";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const unsigned char *id = sqlite3_column_blob(stmt, 0);
		const int id_size = sqlite3_column_bytes(stmt, 0);
		char uuid[33];
		if (human_uuid(&uuid, id, id_size) == -1) {
			error("failed to convert uuid to hex\n");
			response->status = 500;
			goto cleanup;
		}
		// TODO: sign a jwt with id in the payload bay
		response->header_len =
				(size_t)sprintf(response->header, "set-cookie:bearer=%s;Path=/;Max-Age=7200;HttpOnly;\r\n\r\n", uuid);
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

cleanup:
	sqlite3_finalize(stmt);
}

#include "user.h"
#include "bwt.h"
#include "logger.h"
#include "response.h"
#include <sqlite3.h>
#include <string.h>

int find_user_by_id(sqlite3 *database, uint8_t (*user_id)[16], user_t *user) {
	sqlite3_stmt *stmt;

	int status = 404;

	const char *sql = "select id, username, public from user where id = ?";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, *user_id, sizeof(*user_id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		const uint8_t *username = sqlite3_column_text(stmt, 1);
		const size_t username_len = (size_t)sqlite3_column_bytes(stmt, 1);
		const bool public = (bool)sqlite3_column_int(stmt, 2);
		if (id_len != sizeof(user->id)) {
			error("id length does not match buffer\n");
			status = 500;
			goto cleanup;
		}
		if (username_len > sizeof(user->username) - 1) {
			error("username length exceeds buffer\n");
			status = 500;
			goto cleanup;
		}
		memcpy(user->id, id, id_len);
		memcpy(user->username, username, username_len + 1);
		user->public = public;
		status = 0;
	} else if (result == SQLITE_DONE) {
		goto cleanup;
	} else {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to execute statement\n");
		status = 500;
		goto cleanup;
	}

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

int find_user_by_name(sqlite3 *database, char *name, size_t name_len, user_t *user) {
	sqlite3_stmt *stmt;

	int status = 404;

	const char *sql = "select id, username, public from user where username = ?";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		status = 500;
		goto cleanup;
	}

	sqlite3_bind_text(stmt, 1, name, (int)name_len, SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		const uint8_t *username = sqlite3_column_text(stmt, 1);
		const size_t username_len = (size_t)sqlite3_column_bytes(stmt, 1);
		const bool public = (bool)sqlite3_column_int(stmt, 2);
		if (id_len != sizeof(user->id)) {
			error("id length does not match buffer\n");
			status = 500;
			goto cleanup;
		}
		if (username_len > sizeof(user->username) - 1) {
			error("username length exceeds buffer\n");
			status = 500;
			goto cleanup;
		}
		memcpy(user->id, id, id_len);
		memcpy(user->username, username, username_len + 1);
		user->public = public;
		status = 0;
	} else if (result == SQLITE_DONE) {
		goto cleanup;
	} else {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to execute statement\n");
		status = 500;
		goto cleanup;
	}

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

void find_user(sqlite3 *database, bwt_t *bwt, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "select id, username, public from user where id = ?";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, bwt->id, sizeof(bwt->id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		const uint8_t *username = sqlite3_column_text(stmt, 1);
		const size_t username_len = (size_t)sqlite3_column_bytes(stmt, 1);
		const uint8_t public = (uint8_t)sqlite3_column_int(stmt, 2);
		append_body(response, id, id_len);
		append_body(response, username, username_len + 1);
		append_body(response, &public, sizeof(public));
	} else if (result == SQLITE_DONE) {
		error("user %.8x not found\n", *(uint32_t *)bwt->id);
		response->status = 500;
		goto cleanup;
	} else {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to execute statement\n");
		response->status = 500;
		goto cleanup;
	}

	info("user %.8x found himself\n", *(uint32_t *)bwt->id);
	response->status = 200;

	append_header(response, "content-type:application/octet-stream\r\n");
	append_header(response, "content-length:%zu\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

void delete_user(sqlite3 *database, bwt_t *bwt, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "delete from user where id = ?";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, bwt->id, sizeof(bwt->id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result != SQLITE_DONE) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to execute statement\n");
		response->status = 500;
		goto cleanup;
	}

	if (sqlite3_changes(database) == 0) {
		error("user %.8x not found\n", *(uint32_t *)bwt->id);
		response->status = 500;
		goto cleanup;
	}

	info("user %.8x deleted himself\n", *(uint32_t *)bwt->id);
	response->status = 200;

cleanup:
	sqlite3_finalize(stmt);
}

#include "user.h"
#include "bwt.h"
#include "logger.h"
#include "response.h"
#include <sqlite3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint16_t find_user_by_id(sqlite3 *database, uint8_t (*user_id)[16], user_t *user) {
	sqlite3_stmt *stmt;

	uint16_t status = 404;

	const char *sql = "select id, username, visibility from user where id = ?";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
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
		const uint8_t visibility = (uint8_t)sqlite3_column_int(stmt, 2);
		if (id_len != sizeof(user->id)) {
			error("id length %zu does not match buffer length %zu\n", id_len, sizeof(user->id));
			status = 500;
			goto cleanup;
		}
		if (username_len > sizeof(user->username) - 1) {
			error("username length %zu exceeds buffer length %zu\n", username_len, sizeof(user->username) - 1);
			status = 500;
			goto cleanup;
		}
		memcpy(user->id, id, id_len);
		memcpy(user->username, username, username_len + 1);
		user->visibility = visibility;
		status = 0;
	} else if (result == SQLITE_DONE) {
		goto cleanup;
	} else {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		status = 500;
		goto cleanup;
	}

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

uint16_t find_user_by_name(sqlite3 *database, char *name, size_t name_len, user_t *user) {
	sqlite3_stmt *stmt;

	uint16_t status = 404;

	const char *sql = "select id, username, visibility from user where username = ?";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
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
		const uint8_t visibility = (uint8_t)sqlite3_column_int(stmt, 2);
		if (id_len != sizeof(user->id)) {
			error("id length %zu does not match buffer length %zu\n", id_len, sizeof(user->id));
			status = 500;
			goto cleanup;
		}
		if (username_len > sizeof(user->username) - 1) {
			error("username length %zu exceeds buffer length %zu\n", username_len, sizeof(user->username) - 1);
			status = 500;
			goto cleanup;
		}
		memcpy(user->id, id, id_len);
		memcpy(user->username, username, username_len + 1);
		user->visibility = visibility;
		status = 0;
	} else if (result == SQLITE_DONE) {
		goto cleanup;
	} else {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		status = 500;
		goto cleanup;
	}

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

void find_user(sqlite3 *database, bwt_t *bwt, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "select id, username, visibility from user where id = ?";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
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
		const uint8_t visibility = (uint8_t)sqlite3_column_int(stmt, 2);
		append_body(response, id, id_len);
		append_body(response, username, username_len + 1);
		append_body(response, &visibility, sizeof(visibility));
	} else if (result == SQLITE_DONE) {
		error("user %.8x not found\n", *(uint32_t *)bwt->id);
		response->status = 500;
		goto cleanup;
	} else {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
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

void update_user(sqlite3 *database, bwt_t *bwt, user_t *user, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "update user set visibility = ? where id = ?";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_int(stmt, 1, user->visibility);
	sqlite3_bind_blob(stmt, 2, bwt->id, sizeof(bwt->id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result != SQLITE_DONE) {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	if (sqlite3_changes(database) == 0) {
		error("user %.8x not found\n", *(uint32_t *)bwt->id);
		response->status = 500;
		goto cleanup;
	}

	info("user %.8x updated himself\n", *(uint32_t *)bwt->id);
	response->status = 200;

cleanup:
	sqlite3_finalize(stmt);
}

void delete_user(sqlite3 *database, bwt_t *bwt, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "delete from user where id = ?";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, bwt->id, sizeof(bwt->id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result != SQLITE_DONE) {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
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

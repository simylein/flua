#include "friend.h"
#include "bwt.h"
#include "logger.h"
#include "user.h"
#include <sqlite3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint16_t find_friend_by_user_id(sqlite3 *database, bwt_t *bwt, user_t *user, friend_t *friend) {
	sqlite3_stmt *stmt;

	uint16_t status = 404;

	const char *sql = "select id, user_id from friend where id = ? and user_id = ?";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, bwt->id, sizeof(bwt->id), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 2, user->id, sizeof(user->id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		const uint8_t *id = sqlite3_column_blob(stmt, 0);
		const size_t id_len = (size_t)sqlite3_column_bytes(stmt, 0);
		const uint8_t *user_id = sqlite3_column_blob(stmt, 1);
		const size_t user_id_len = (size_t)sqlite3_column_bytes(stmt, 1);
		if (id_len != sizeof(friend->id)) {
			error("id length %zu does not match buffer length %zu\n", id_len, sizeof(friend->id));
			status = 500;
			goto cleanup;
		}
		if (user_id_len > sizeof(friend->user_id)) {
			error("user id length %zu does not match buffer length %zu\n", user_id_len, sizeof(friend->user_id));
			status = 500;
			goto cleanup;
		}
		memcpy(friend->id, id, id_len);
		memcpy(friend->user_id, user_id, user_id_len);
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

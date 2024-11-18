#include "database.h"
#include "logger.h"
#include "response.h"
#include <sqlite3.h>
#include <string.h>

void find_flights(char *year, int user_id, Response *response) {
	info("finding flights for %s\n", year);

	sqlite3_stmt *stmt;

	const char *sql = "select starts_at, ends_at from flight "
										"where user_id = ? and strftime('%Y', datetime(starts_at, 'unixepoch')) = ? "
										"order by starts_at desc";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, 0) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_int(stmt, 1, user_id);
	sqlite3_bind_text(stmt, 2, year, -1, SQLITE_STATIC);

	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const int64_t starts_at = sqlite3_column_int64(stmt, 0);
			const int64_t ends_at = sqlite3_column_int64(stmt, 1);
			if (response->body_len + sizeof(starts_at) + sizeof(ends_at) > sizeof(response->body)) {
				error("body length exceeds buffer\n");
				response->status = 206;
				goto partial;
			}
			memcpy(response->body + response->body_len, &starts_at, sizeof(starts_at));
			response->body_len += sizeof(starts_at);
			memcpy(response->body + response->body_len, &ends_at, sizeof(ends_at));
			response->body_len += sizeof(ends_at);
		} else if (result == SQLITE_DONE) {
			break;
		} else {
			error("%s\n", sqlite3_errmsg(database));
			error("failed to execute statement\n");
			response->status = 500;
			goto cleanup;
		}
	}

partial:
	sprintf(response->header, "application/octet-stream\r\ncontent-length:%zu\r\n\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

void find_flight_years(int user_id, Response *response) {
	info("finding flight years\n");

	sqlite3_stmt *stmt;

	const char *sql = "select distinct strftime('%Y', datetime(starts_at, 'unixepoch')) as year from flight "
										"where user_id = ? "
										"order by year desc";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, 0) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_int(stmt, 1, user_id);

	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const int year = sqlite3_column_int(stmt, 0);
			if (response->body_len + sizeof(year) > sizeof(response->body)) {
				error("body length exceeds buffer\n");
				response->status = 206;
				goto partial;
			}
			memcpy(response->body + response->body_len, &year, sizeof(year));
			response->body_len += sizeof(year);
		} else if (result == SQLITE_DONE) {
			break;
		} else {
			error("%s\n", sqlite3_errmsg(database));
			error("failed to execute statement\n");
			response->status = 500;
			goto cleanup;
		}
	}

partial:
	sprintf(response->header, "application/octet-stream\r\ncontent-length:%zu\r\n\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

#include "database.h"
#include "format.h"
#include "logger.h"
#include "response.h"
#include <arpa/inet.h>
#include <sqlite3.h>
#include <string.h>

void find_flights(char *user_uuid, char *year, Response *response) {
	info("finding flights for %s\n", year);

	sqlite3_stmt *stmt;

	const char *sql = "select starts_at, ends_at from flight "
										"where user_id = ? and strftime('%Y', datetime(starts_at, 'unixepoch')) = ? "
										"order by starts_at desc";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	unsigned char user_id[16];
	if (hex_to_bin(user_id, sizeof(user_id), user_uuid, strlen(user_uuid)) == -1) {
		error("failed to convert uuid to binary\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, user_id, sizeof(user_id), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, year, -1, SQLITE_STATIC);

	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const u_int64_t starts_at = ntohll(sqlite3_column_int64(stmt, 0));
			const u_int64_t ends_at = ntohll(sqlite3_column_int64(stmt, 1));
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
			response->status = 200;
			break;
		} else {
			error("%s\n", sqlite3_errmsg(database));
			error("failed to execute statement\n");
			response->status = 500;
			goto cleanup;
		}
	}

partial:
	response->header_len = (size_t)sprintf(
			response->header, "content-type:application/octet-stream\r\ncontent-length:%zu\r\n\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

void find_flight_years(char *user_uuid, Response *response) {
	info("finding flight years\n");

	sqlite3_stmt *stmt;

	const char *sql = "select distinct strftime('%Y', datetime(starts_at, 'unixepoch')) as year from flight "
										"where user_id = ? "
										"order by year desc";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	unsigned char user_id[16];
	if (hex_to_bin(user_id, sizeof(user_id), user_uuid, strlen(user_uuid)) == -1) {
		error("failed to convert uuid to binary\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, user_id, sizeof(user_id), SQLITE_STATIC);

	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const u_int16_t year = ntohs(sqlite3_column_int(stmt, 0));
			if (response->body_len + sizeof(year) > sizeof(response->body)) {
				error("body length exceeds buffer\n");
				response->status = 206;
				goto partial;
			}
			memcpy(response->body + response->body_len, &year, sizeof(year));
			response->body_len += sizeof(year);
		} else if (result == SQLITE_DONE) {
			response->status = 200;
			break;
		} else {
			error("%s\n", sqlite3_errmsg(database));
			error("failed to execute statement\n");
			response->status = 500;
			goto cleanup;
		}
	}

partial:
	response->header_len = (size_t)sprintf(
			response->header, "content-type:application/octet-stream\r\ncontent-length:%zu\r\n\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

void create_flight(char *user_uuid, char *hex_hash, u_int64_t starts_at, u_int64_t ends_at, Response *response) {
	info("creating new flight\n");

	sqlite3_stmt *stmt;

	const char *sql = "insert into flight (id, hash, starts_at, ends_at, user_id) values (randomblob(16), ?, ?, ?, ?)";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	unsigned char hash[16];
	if (hex_to_bin(hash, sizeof(hash), hex_hash, strlen(hex_hash)) == -1) {
		error("failed to convert hash to binary\n");
		response->status = 500;
		goto cleanup;
	}

	unsigned char user_id[16];
	if (hex_to_bin(user_id, sizeof(user_id), user_uuid, strlen(user_uuid)) == -1) {
		error("failed to convert uuid to binary\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, hash, sizeof(hash), SQLITE_STATIC);
	sqlite3_bind_int64(stmt, 2, (int64_t)starts_at);
	sqlite3_bind_int64(stmt, 3, (int64_t)ends_at);
	sqlite3_bind_blob(stmt, 4, user_id, sizeof(user_id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_DONE) {
		response->status = 201;
		goto cleanup;
	} else if (result == SQLITE_CONSTRAINT) {
		warn("flight %.8s already exists\n", hex_hash);
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

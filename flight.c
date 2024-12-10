#include "flight.h"
#include "bwt.h"
#include "endian.h"
#include "format.h"
#include "logger.h"
#include "response.h"
#include <sqlite3.h>
#include <stdint.h>
#include <string.h>

void find_years(sqlite3 *database, uint8_t (*user_id)[16], response_t *response) {
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

	sqlite3_bind_blob(stmt, 1, *user_id, sizeof(*user_id), SQLITE_STATIC);

	size_t rows = 0;
	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const uint16_t year = htons((uint16_t)sqlite3_column_int(stmt, 0));
			if (response->body_len + sizeof(year) > sizeof(response->body)) {
				error("body length exceeds buffer\n");
				response->status = 206;
				goto partial;
			}
			append_body(response, &year, sizeof(year));
			rows++;
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
	info("found %zu years\n", rows);

	append_header(response, "content-type:application/octet-stream\r\n");
	append_header(response, "content-length:%zu\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

void find_flights(sqlite3 *database, uint8_t (*user_id)[16], char *year, response_t *response) {
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

	sqlite3_bind_blob(stmt, 1, *user_id, sizeof(*user_id), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, year, -1, SQLITE_STATIC);

	size_t rows = 0;
	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const uint64_t starts_at = htonll((uint64_t)sqlite3_column_int64(stmt, 0));
			const uint64_t ends_at = htonll((uint64_t)sqlite3_column_int64(stmt, 1));
			if (response->body_len + sizeof(starts_at) + sizeof(ends_at) > sizeof(response->body)) {
				error("body length exceeds buffer\n");
				response->status = 206;
				goto partial;
			}
			append_body(response, &starts_at, sizeof(starts_at));
			append_body(response, &ends_at, sizeof(ends_at));
			rows++;
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
	info("found %zu flights in %s\n", rows, year);

	append_header(response, "content-type:application/octet-stream\r\n");
	append_header(response, "content-length:%zu\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

void create_flight(sqlite3 *database, bwt_t *bwt, flight_t *flight, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "insert into flight (id, hash, starts_at, ends_at, user_id) values (randomblob(16), ?, ?, ?, ?)";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to prepare statement\n");
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, flight->hash, sizeof(flight->hash), SQLITE_STATIC);
	sqlite3_bind_int64(stmt, 2, (int64_t)flight->starts_at);
	sqlite3_bind_int64(stmt, 3, (int64_t)flight->ends_at);
	sqlite3_bind_blob(stmt, 4, bwt->id, sizeof(bwt->id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_DONE) {
		response->status = 201;
	} else if (result == SQLITE_CONSTRAINT) {
		warn("flight %.8x already exists\n", *(uint32_t *)flight->hash);
		response->status = 409;
		goto cleanup;
	} else {
		error("%s\n", sqlite3_errmsg(database));
		error("failed to execute statement\n");
		response->status = 500;
		goto cleanup;
	}

	info("created flight %.8x\n", *(uint32_t *)flight->hash);

cleanup:
	sqlite3_finalize(stmt);
}

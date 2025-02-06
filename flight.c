#include "flight.h"
#include "bwt.h"
#include "endian.h"
#include "format.h"
#include "logger.h"
#include "response.h"
#include "utils.h"
#include <sqlite3.h>
#include <stdint.h>
#include <string.h>

void find_years(sqlite3 *database, uint8_t (*user_id)[16], response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "select distinct strftime('%Y', datetime(starts_at, 'unixepoch')) as year from flight "
										"where user_id = ? "
										"order by year desc";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, *user_id, sizeof(*user_id), SQLITE_STATIC);

	size_t rows = 0;
	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const uint16_t year = htons((uint16_t)sqlite3_column_int(stmt, 0));
			if (response->body_len + sizeof(year) > sizeof(*response->body)) {
				error("body length %zu exceeds buffer length %zu\n", response->body_len, sizeof(*response->body));
				response->status = 206;
				goto partial;
			}
			append_body(response, &year, sizeof(year));
			rows++;
		} else if (result == SQLITE_DONE) {
			response->status = 200;
			break;
		} else {
			error("failed to execute statement because %s\n", sqlite3_errmsg(database));
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

void find_flights(sqlite3 *database, uint8_t (*user_id)[16], char *year, size_t year_len, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "select starts_at, ends_at, altitude_bins, thermal_bins, speed_bins, glide_bins from flight "
										"where user_id = ? and strftime('%Y', datetime(starts_at, 'unixepoch')) = ? "
										"order by starts_at desc";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, *user_id, sizeof(*user_id), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, year, (int)year_len, SQLITE_STATIC);

	size_t rows = 0;
	while (1) {
		int result = sqlite3_step(stmt);
		if (result == SQLITE_ROW) {
			const uint64_t starts_at = (uint64_t)sqlite3_column_int64(stmt, 0);
			const uint64_t ends_at = (uint64_t)sqlite3_column_int64(stmt, 1);
			const uint16_t(*altitude_bins)[5] = (const uint16_t(*)[5])sqlite3_column_blob(stmt, 2);
			const size_t altitude_bins_len = (size_t)sqlite3_column_bytes(stmt, 2);
			const uint16_t(*thermal_bins)[5] = (const uint16_t(*)[5])sqlite3_column_blob(stmt, 3);
			const size_t thermal_bins_len = (size_t)sqlite3_column_bytes(stmt, 3);
			const uint16_t(*speed_bins)[5] = (const uint16_t(*)[5])sqlite3_column_blob(stmt, 4);
			const size_t speed_bins_len = (size_t)sqlite3_column_bytes(stmt, 4);
			const uint16_t(*glide_bins)[5] = (const uint16_t(*)[5])sqlite3_column_blob(stmt, 5);
			const size_t glide_bins_len = (size_t)sqlite3_column_bytes(stmt, 5);
			if (altitude_bins_len != sizeof(*altitude_bins)) {
				error("altitude bins length %zu does not match buffer length %zu\n", altitude_bins_len, sizeof(*altitude_bins));
				response->status = 500;
				goto cleanup;
			}
			if (thermal_bins_len != sizeof(*thermal_bins)) {
				error("thermal bins length %zu does not match buffer length %zu\n", thermal_bins_len, sizeof(*thermal_bins));
				response->status = 500;
				goto cleanup;
			}
			if (speed_bins_len != sizeof(*speed_bins)) {
				error("speed bins length %zu does not match buffer length %zu\n", speed_bins_len, sizeof(*speed_bins));
				response->status = 500;
				goto cleanup;
			}
			if (glide_bins_len != sizeof(*glide_bins)) {
				error("glide bins length %zu does not match buffer length %zu\n", glide_bins_len, sizeof(*glide_bins));
				response->status = 500;
				goto cleanup;
			}
			if (response->body_len + sizeof(starts_at) + sizeof(ends_at) + sizeof(*altitude_bins) + sizeof(*thermal_bins) >
					sizeof(*response->body)) {
				error("body length %zu exceeds buffer length %zu\n", response->body_len, sizeof(*response->body));
				response->status = 206;
				goto partial;
			}
			const uint64_t n_starts_at = htonll(starts_at);
			const uint64_t n_ends_at = htonll(ends_at);
			const uint8_t starts_at_bytes = significant_bytes(starts_at);
			const uint8_t ends_at_bytes = significant_bytes(ends_at);
			const uint8_t altitude_bins_bytes[5] = {
					significant_bytes((*altitude_bins)[0]), significant_bytes((*altitude_bins)[1]),
					significant_bytes((*altitude_bins)[2]), significant_bytes((*altitude_bins)[3]),
					significant_bytes((*altitude_bins)[4]),
			};
			const uint8_t thermal_bins_bytes[5] = {
					significant_bytes((*thermal_bins)[0]), significant_bytes((*thermal_bins)[1]), significant_bytes((*thermal_bins)[2]),
					significant_bytes((*thermal_bins)[3]), significant_bytes((*thermal_bins)[4]),
			};
			const uint8_t speed_bins_bytes[5] = {
					significant_bytes((*speed_bins)[0]), significant_bytes((*speed_bins)[1]), significant_bytes((*speed_bins)[2]),
					significant_bytes((*speed_bins)[3]), significant_bytes((*speed_bins)[4]),
			};
			const uint8_t glide_bins_bytes[5] = {
					significant_bytes((*glide_bins)[0]), significant_bytes((*glide_bins)[1]), significant_bytes((*glide_bins)[2]),
					significant_bytes((*glide_bins)[3]), significant_bytes((*glide_bins)[4]),
			};
			const uint64_t leading_bytes = ((uint64_t)starts_at_bytes << 60) | ((uint64_t)ends_at_bytes << 56) |
																		 ((uint64_t)altitude_bins_bytes[0] << 54) | ((uint64_t)altitude_bins_bytes[1] << 52) |
																		 ((uint64_t)altitude_bins_bytes[2] << 50) | ((uint64_t)altitude_bins_bytes[3] << 48) |
																		 ((uint64_t)altitude_bins_bytes[4] << 46) | ((uint64_t)thermal_bins_bytes[0] << 44) |
																		 ((uint64_t)thermal_bins_bytes[1] << 42) | ((uint64_t)thermal_bins_bytes[2] << 40) |
																		 ((uint64_t)thermal_bins_bytes[3] << 38) | ((uint64_t)thermal_bins_bytes[4] << 36) |
																		 ((uint64_t)speed_bins_bytes[0] << 34) | ((uint64_t)speed_bins_bytes[1] << 32) |
																		 ((uint64_t)speed_bins_bytes[2] << 30) | ((uint64_t)speed_bins_bytes[3] << 28) |
																		 ((uint64_t)speed_bins_bytes[4] << 26) | ((uint64_t)glide_bins_bytes[0] << 24) |
																		 ((uint64_t)glide_bins_bytes[1] << 22) | ((uint64_t)glide_bins_bytes[2] << 20) |
																		 ((uint64_t)glide_bins_bytes[3] << 18) | ((uint64_t)glide_bins_bytes[4] << 16);
			const uint64_t n_leading_bytes = htonll(leading_bytes);
			append_body(response, &n_leading_bytes, sizeof(leading_bytes) - 2);
			append_body(response, ((uint8_t *)&n_starts_at) + (8 - starts_at_bytes), starts_at_bytes);
			append_body(response, ((uint8_t *)&n_ends_at) + (8 - ends_at_bytes), ends_at_bytes);
			for (uint8_t index = 0; index < 5; index++) {
				append_body(response, ((uint8_t *)&(*altitude_bins)[index]) + (2 - altitude_bins_bytes[index]),
										altitude_bins_bytes[index]);
			}
			for (uint8_t index = 0; index < 5; index++) {
				append_body(response, ((uint8_t *)&(*thermal_bins)[index]) + (2 - thermal_bins_bytes[index]),
										thermal_bins_bytes[index]);
			}
			for (uint8_t index = 0; index < 5; index++) {
				append_body(response, ((uint8_t *)&(*speed_bins)[index]) + (2 - speed_bins_bytes[index]), speed_bins_bytes[index]);
			}
			for (uint8_t index = 0; index < 5; index++) {
				append_body(response, ((uint8_t *)&(*glide_bins)[index]) + (2 - glide_bins_bytes[index]), glide_bins_bytes[index]);
			}
			rows++;
		} else if (result == SQLITE_DONE) {
			response->status = 200;
			break;
		} else {
			error("failed to execute statement because %s\n", sqlite3_errmsg(database));
			response->status = 500;
			goto cleanup;
		}
	}

partial:
	info("found %zu flights in %.*s\n", rows, (int)year_len, year);

	append_header(response, "content-type:application/octet-stream\r\n");
	append_header(response, "content-length:%zu\r\n", response->body_len);

cleanup:
	sqlite3_finalize(stmt);
}

void create_flight(sqlite3 *database, bwt_t *bwt, flight_t *flight, response_t *response) {
	sqlite3_stmt *stmt;

	const char *sql = "insert into flight (id, hash, "
										"starts_at, ends_at, "
										"altitude_bins, altitude_min, altitude_max, "
										"thermal_bins, max_climb, max_sink, "
										"speed_bins, speed_avg, speed_max, "
										"glide_bins, distance_flown, "
										"user_id) "
										"values (randomblob(16), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	sqlite3_bind_blob(stmt, 1, flight->hash, sizeof(*flight->hash), SQLITE_STATIC);
	sqlite3_bind_int64(stmt, 2, (int64_t)*flight->starts_at);
	sqlite3_bind_int64(stmt, 3, (int64_t)*flight->ends_at);
	sqlite3_bind_blob(stmt, 4, flight->altitude_bins, sizeof(*flight->altitude_bins), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 5, (int32_t)*flight->altitude_min);
	sqlite3_bind_int(stmt, 6, (int32_t)*flight->altitude_max);
	sqlite3_bind_blob(stmt, 7, flight->thermal_bins, sizeof(*flight->thermal_bins), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 8, (int32_t)*flight->max_climb);
	sqlite3_bind_int(stmt, 9, (int32_t)*flight->max_sink);
	sqlite3_bind_blob(stmt, 10, flight->speed_bins, sizeof(*flight->speed_bins), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 11, (int32_t)*flight->speed_avg);
	sqlite3_bind_int(stmt, 12, (int32_t)*flight->speed_max);
	sqlite3_bind_blob(stmt, 13, flight->glide_bins, sizeof(*flight->glide_bins), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 14, (int32_t)*flight->distance_flown);
	sqlite3_bind_blob(stmt, 15, bwt->id, sizeof(bwt->id), SQLITE_STATIC);

	int result = sqlite3_step(stmt);
	if (result == SQLITE_DONE) {
		response->status = 201;
	} else if (result == SQLITE_CONSTRAINT) {
		warn("flight %.8x already exists\n", *(uint32_t *)flight->hash);
		response->status = 409;
		goto cleanup;
	} else {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		response->status = 500;
		goto cleanup;
	}

	info("created flight %.8x\n", *(uint32_t *)flight->hash);

cleanup:
	sqlite3_finalize(stmt);
}

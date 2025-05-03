#include "logger.h"
#include <sqlite3.h>
#include <stdlib.h>

int flow_init(sqlite3 *database) {
	int status;

	sqlite3_stmt *stmt;

	const char *sql = "create table flow "
										"(id blob not null primary key, "
										"timestamp int not null, "
										"method text not null, "
										"pathname text not null, "
										"search text not null, "
										"status int not null, "
										"duration int not null, "
										"bytes_received int not null, "
										"bytes_sent int not null)";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	info("created flow table\n");
	status = 0;

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

int user_init(sqlite3 *database) {
	int status;

	sqlite3_stmt *stmt;

	const char *sql = "create table user"
										"(id blob not null primary key, "
										"username text not null unique, "
										"password blob not null, "
										"visibility int not null)";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	char *message = NULL;
	if (sqlite3_exec(database, "create index index_username on user(username)", NULL, NULL, &message)) {
		error("failed to index user table because %s\n", message);
		sqlite3_free(message);
		status = -1;
		goto cleanup;
	};

	info("created user table\n");
	status = 0;

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

int friend_init(sqlite3 *database) {
	int status;

	sqlite3_stmt *stmt;

	const char *sql = "create table friend"
										"(id blob not null, "
										"user_id blob not null, "
										"foreign key (id) references user(id) on delete cascade, "
										"foreign key (user_id) references user(id) on delete cascade)";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	char *message = NULL;
	if (sqlite3_exec(database, "create index index_id_user_id on friend(id, user_id)", NULL, NULL, &message)) {
		error("failed to index friend table because %s\n", message);
		sqlite3_free(message);
		status = -1;
		goto cleanup;
	};

	info("created friend table\n");
	status = 0;

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

int flight_init(sqlite3 *database) {
	int status;

	sqlite3_stmt *stmt;

	const char *sql = "create table flight"
										"(id blob not null primary key, "
										"hash blob not null unique, "
										"starts_at datetime not null, "
										"ends_at datetime not null, "
										"altitude_bins blob not null, "
										"altitude_min int not null, "
										"altitude_max int not null, "
										"thermal_bins blob not null, "
										"max_climb int not null, "
										"max_sink int not null, "
										"speed_bins blob not null, "
										"speed_avg int not null, "
										"speed_max int not null, "
										"glide_bins blob not null, "
										"distance_flown int not null, "
										"user_id blob not null, "
										"foreign key (user_id) references user(id) on delete cascade)";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		status = -1;
		goto cleanup;
	}

	char *message = NULL;
	if (sqlite3_exec(database, "create index index_user_id_starts_at on flight(user_id, starts_at)", NULL, NULL, &message)) {
		error("failed to index user table because %s\n", message);
		sqlite3_free(message);
		status = -1;
		goto cleanup;
	};

	info("created flight table\n");
	status = 0;

cleanup:
	sqlite3_finalize(stmt);
	return status;
}

int init(sqlite3 *database) {
	if (flow_init(database) != 0) {
		return -1;
	}
	if (user_init(database) != 0) {
		return -1;
	}
	if (friend_init(database) != 0) {
		return -1;
	}
	if (flight_init(database) != 0) {
		return -1;
	}

	return 0;
}

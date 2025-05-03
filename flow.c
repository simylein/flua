#include "flow.h"
#include "config.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "thread.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

void *flowing(void *args) {
	arg_t *arg = (arg_t *)args;

	while (true) {
		pthread_mutex_lock(&stash.lock);

		while (stash.size == 0) {
			pthread_cond_wait(&stash.filled, &stash.lock);
		}

		flow_t flow = stash.flows[stash.head];
		stash.head = (uint8_t)((stash.head + 1) % stash_size);
		stash.size--;
		pthread_mutex_unlock(&stash.lock);

		create_flow(arg->database, &flow);
	}
}

void stash_flow(time_t timestamp, request_t *request, response_t *response, struct timespec *start, struct timespec *stop,
								ssize_t bytes_received, ssize_t bytes_sent) {
	pthread_mutex_lock(&stash.lock);

	if (stash.size >= stash_size) {
		warn("dropping flow because stash size %hhu is full\n", stash.size);
		goto cleanup;
	}

	time_t start_nanoseconds = start->tv_sec * 1000000000 + start->tv_nsec;
	time_t stop_nanoseconds = stop->tv_sec * 1000000000 + stop->tv_nsec;
	time_t nanoseconds = stop_nanoseconds - start_nanoseconds;

	stash.flows[stash.tail].timestamp = timestamp;
	memcpy(stash.flows[stash.tail].method, request->method, request->method_len);
	stash.flows[stash.tail].method_len = request->method_len;
	memcpy(stash.flows[stash.tail].pathname, request->pathname, request->pathname_len);
	stash.flows[stash.tail].pathname_len = request->pathname_len;
	memcpy(stash.flows[stash.tail].search, request->search, request->search_len);
	stash.flows[stash.tail].search_len = request->search_len;
	stash.flows[stash.tail].status = response->status;
	stash.flows[stash.tail].duration = nanoseconds;
	stash.flows[stash.tail].bytes_received = bytes_received;
	stash.flows[stash.tail].bytes_sent = bytes_sent;
	stash.tail = (uint8_t)((stash.tail + 1) % stash_size);
	stash.size++;

	pthread_cond_signal(&stash.filled);

cleanup:
	pthread_mutex_unlock(&stash.lock);
}

void create_flow(sqlite3 *database, flow_t *flow) {
	sqlite3_stmt *stmt;

	const char *sql = "insert into flow "
										"(id, timestamp, "
										"method, pathname, search, "
										"status, duration, "
										"bytes_received, bytes_sent)"
										"values (randomblob(16), ?, ?, ?, ?, ?, ?, ?, ?)";
	debug("%s\n", sql);

	if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) {
		error("failed to prepare statement because %s\n", sqlite3_errmsg(database));
		goto cleanup;
	}

	sqlite3_bind_int64(stmt, 1, flow->timestamp);
	sqlite3_bind_text(stmt, 2, flow->method, flow->method_len, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, flow->pathname, flow->pathname_len, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, flow->search, flow->search_len, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 5, flow->status);
	sqlite3_bind_int64(stmt, 6, flow->duration);
	sqlite3_bind_int64(stmt, 7, flow->bytes_received);
	sqlite3_bind_int64(stmt, 8, flow->bytes_sent);

	int result = sqlite3_step(stmt);
	if (result != SQLITE_DONE) {
		error("failed to execute statement because %s\n", sqlite3_errmsg(database));
		goto cleanup;
	}

cleanup:
	sqlite3_finalize(stmt);
}

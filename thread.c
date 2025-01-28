#include "thread.h"
#include "app.h"
#include "config.h"
#include "error.h"
#include "logger.h"
#include <errno.h>
#include <sqlite3.h>
#include <stdbool.h>

queue_t queue = {
		.front = 0,
		.back = 0,
		.size = 0,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.filled = PTHREAD_COND_INITIALIZER,
		.available = PTHREAD_COND_INITIALIZER,
};

thread_pool_t thread_pool = {
		.size = 0,
		.load = 0,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.available = PTHREAD_COND_INITIALIZER,
};

int spawn(worker_t *workers, uint8_t index, void (*logger)(const char *message, ...) __attribute__((format(printf, 1, 2)))) {
	workers[index].arg.id = index;
	trace("spawning worker thread %hu\n", index);

	int db_error = sqlite3_open_v2(database_file, &workers[index].arg.database, SQLITE_OPEN_READWRITE, NULL);
	if (db_error != SQLITE_OK) {
		logger("failed to open %s because %s\n", database_file, sqlite3_errmsg(workers[index].arg.database));
		return -1;
	}

	int exec_error = sqlite3_exec(workers[index].arg.database, "pragma foreign_keys = on;", NULL, NULL, NULL);
	if (exec_error != SQLITE_OK) {
		logger("failed to enforce foreign key constraints because %s\n", sqlite3_errmsg(workers[index].arg.database));
		return -1;
	}

	sqlite3_busy_timeout(workers[index].arg.database, database_timeout);

	int spawn_error = pthread_create(&workers[index].thread, NULL, thread, (void *)&workers[index].arg);
	if (spawn_error != 0) {
		errno = spawn_error;
		logger("failed to spawn worker thread %hu because %s\n", workers[index].arg.id, errno_str());
		return -1;
	}

	int detach_error = pthread_detach(workers[index].thread);
	if (detach_error != 0) {
		errno = detach_error;
		logger("failed to detach worker thread %hu because %s\n", workers[index].arg.id, errno_str());
		return -1;
	}

	return 0;
}

void *thread(void *args) {
	bool exit = false;
	arg_t *arg = (arg_t *)args;

	while (true) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size == 0) {
			pthread_cond_wait(&queue.filled, &queue.lock);
		}

		task_t task = queue.tasks[queue.front];
		queue.front = (uint8_t)((queue.front + 1) % (queue_size));
		queue.size--;
		trace("worker thread %hu decreased queue size to %hu\n", arg->id, queue.size);
		pthread_cond_signal(&queue.available);
		pthread_mutex_unlock(&queue.lock);

		pthread_mutex_lock(&thread_pool.lock);
		thread_pool.load++;
		trace("worker thread %hu increased thread pool load to %hu\n", arg->id, thread_pool.load);
		pthread_mutex_unlock(&thread_pool.lock);

		handle(arg->database, &task.client_sock, &task.client_addr);

		pthread_mutex_lock(&thread_pool.lock);
		thread_pool.load--;
		trace("worker thread %hu decreased thread pool load to %hu\n", arg->id, thread_pool.load);
		if (thread_pool.load <= thread_pool.size / 2 && arg->id >= least_workers && arg->id + 1 == thread_pool.size) {
			if (sqlite3_close_v2(arg->database) != SQLITE_OK) {
				error("failed to close %s because %s\n", database_file, sqlite3_errmsg(arg->database));
			}
			thread_pool.size--;
			exit = true;
		}
		pthread_cond_signal(&thread_pool.available);
		pthread_mutex_unlock(&thread_pool.lock);

		if (exit == true) {
			trace("killing worker thread %hu\n", arg->id);
			pthread_exit(0);
		}
	}
}

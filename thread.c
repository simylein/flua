#include "thread.h"
#include "app.h"
#include "config.h"
#include "error.h"
#include "logger.h"
#include <errno.h>
#include <sqlite3.h>

queue_t queue = {
		.front = 0,
		.back = 0,
		.size = 0,
		.load = 0,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.filled = PTHREAD_COND_INITIALIZER,
		.available = PTHREAD_COND_INITIALIZER,
};

int spawn(arg_t *args, pthread_t *threads, size_t index) {
	args[index].id = index;
	trace("spawning worker thread %zu\n", index);

	int db_error = sqlite3_open_v2(database_file, &args[index].database, SQLITE_OPEN_READWRITE, NULL);
	if (db_error != SQLITE_OK) {
		fatal("failed to open %s because %s\n", database_file, sqlite3_errmsg(args[index].database));
		return -1;
	}

	int exec_error = sqlite3_exec(args[index].database, "pragma foreign_keys = on;", NULL, NULL, NULL);
	if (exec_error != SQLITE_OK) {
		fatal("failed to enforce foreign key constraints because %s\n", sqlite3_errmsg(args[index].database));
		return -1;
	}

	sqlite3_busy_timeout(args[index].database, database_timeout);

	int spawn_error = pthread_create(&threads[index], NULL, thread, (void *)&args[index]);
	if (spawn_error != 0) {
		errno = spawn_error;
		fatal("failed to spawn worker thread %zu because %s\n", args[index].id, errno_str());
		return -1;
	}

	return 0;
}

void *thread(void *args) {
	arg_t *arg = (arg_t *)args;

	while (1) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size == 0) {
			pthread_cond_wait(&queue.filled, &queue.lock);
		}

		task_t task = queue.tasks[queue.front];
		queue.front = (queue.front + 1) % (queue_size);
		queue.size--;
		trace("worker thread %zu decreased queue size to %zu\n", arg->id, queue.size);
		queue.load++;
		trace("worker thread %zu increased queue load to %zu\n", arg->id, queue.load);

		pthread_cond_signal(&queue.available);
		pthread_mutex_unlock(&queue.lock);

		handle(arg->database, &task.client_sock, &task.client_addr);

		pthread_mutex_lock(&queue.lock);
		queue.load--;
		trace("worker thread %zu decreased queue load to %zu\n", arg->id, queue.load);
		pthread_mutex_unlock(&queue.lock);
	}
}

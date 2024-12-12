#include "thread.h"
#include "app.h"
#include "config.h"
#include "error.h"
#include "logger.h"
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

void *thread(void *args) {
	struct arg_t *arg = (struct arg_t *)args;

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

#include "thread.h"
#include "app.h"
#include "config.h"
#include "error.h"
#include "logger.h"

Queue queue = {
		.front = 0,
		.back = 0,
		.size = 0,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.filled = PTHREAD_COND_INITIALIZER,
		.available = PTHREAD_COND_INITIALIZER,
};

void *thread(void *arg) {
	int id = (int)(intptr_t)arg;

	while (1) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size == 0) {
			pthread_cond_wait(&queue.filled, &queue.lock);
		}

		Task task = queue.tasks[queue.front];
		queue.front = (queue.front + 1) % ((size_t)workers * 2);
		queue.size--;
		trace("worker thread %d decreased queue size to %zu\n", id, queue.size);

		pthread_cond_signal(&queue.available);
		pthread_mutex_unlock(&queue.lock);

		handle(&task.client_sock, &task.client_addr);
	}
}

#include "config.h"
#include "error.h"
#include "logger.h"
#include "thread.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int cf_errors = configure(argc, argv);
	if (cf_errors != 0) {
		fatal("config contains %d errors\n", cf_errors);
		exit(1);
	}

	int log_errors = logfiles("req.log", "res.log", "trace.log", "debug.log", "info.log", "warn.log", "error.log", "fatal.log");
	if (log_errors != 0) {
		fatal("failed to open %d log files\n", log_errors);
		exit(1);
	}

	info("using database %s\n", database_file);

	int server_sock;
	struct sockaddr_in server_addr;

	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fatal("failed to create socket because %s\n", errno_str());
		exit(1);
	}

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) == -1) {
		fatal("failed to set socket options because %s\n", errno_str());
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address);
	server_addr.sin_port = htons(port);

	if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		fatal("failed to bind to socket because %s\n", errno_str());
		exit(1);
	}

	if (listen(server_sock, backlog) == -1) {
		fatal("failed to listen on socket because %s\n", errno_str());
		exit(1);
	}

	info("listening on %s:%d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

	thread_pool.workers = malloc(most_workers * sizeof(worker_t));
	if (thread_pool.workers == NULL) {
		fatal("failed to allocate %zu bytes for workers because %s\n", most_workers * sizeof(worker_t), errno_str());
		exit(1);
	}

	queue.tasks = malloc(queue_size * sizeof(task_t));
	if (queue.tasks == NULL) {
		fatal("failed to allocate %zu bytes for tasks because %s\n", queue_size * sizeof(task_t), errno_str());
		exit(1);
	}

	for (uint8_t index = 0; index < least_workers; index++) {
		if (spawn(thread_pool.workers, thread_pool.size, &fatal) == -1) {
			exit(1);
		}
		thread_pool.size++;
	}

	while (true) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size >= queue_size) {
			warn("waiting for queue size to decrease\n");
			pthread_cond_wait(&queue.available, &queue.lock);
		}

		pthread_mutex_unlock(&queue.lock);

		pthread_mutex_lock(&thread_pool.lock);

		if (thread_pool.load >= thread_pool.size) {
			warn("all worker threads currently busy\n");
			uint8_t new_size = thread_pool.size + 1;
			if (new_size <= most_workers && spawn(thread_pool.workers, thread_pool.size, &error) == 0) {
				info("scaled threads from %hu to %hu\n", thread_pool.size, new_size);
				thread_pool.size = new_size;
			}
		}

		pthread_mutex_unlock(&thread_pool.lock);

		struct sockaddr_in client_addr;
		int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &(socklen_t){sizeof(client_addr)});

		if (client_sock == -1) {
			error("failed to accept client because %s\n", errno_str());
			continue;
		}

		pthread_mutex_lock(&queue.lock);

		queue.tasks[queue.back].client_sock = client_sock;
		memcpy(&queue.tasks[queue.back].client_addr, &client_addr, sizeof(client_addr));
		queue.back = (queue.back + 1) % (queue_size);
		queue.size++;
		trace("main thread increased queue size to %hu\n", queue.size);

		pthread_cond_signal(&queue.filled);
		pthread_mutex_unlock(&queue.lock);
	}
}

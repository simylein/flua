#include "config.h"
#include "error.h"
#include "logger.h"
#include "thread.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

void scale(arg_t **args, pthread_t **threads, uint8_t *workers, uint8_t new_workers) {
	arg_t *new_args = realloc(*args, new_workers * sizeof(arg_t));
	if (new_args == NULL) {
		error("failed to reallocate %zu bytes for args because %s\n", new_workers * sizeof(arg_t), errno_str());
		return;
	}
	*args = new_args;

	pthread_t *new_threads = realloc(*threads, new_workers * sizeof(pthread_t));
	if (new_threads == NULL) {
		error("failed to reallocate %zu bytes for threads because %s\n", new_workers * sizeof(pthread_t), errno_str());
		return;
	}
	*threads = new_threads;

	if (spawn(*args, *threads, *workers) == -1) {
		return;
	}

	info("scaled threads from %hu to %hu\n", *workers, new_workers);
	*workers = new_workers;
}

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

	uint8_t workers = least_workers;

	arg_t *args = malloc(workers * sizeof(arg_t));
	if (args == NULL) {
		fatal("failed to allocate %zu bytes for args because %s\n", workers * sizeof(arg_t), errno_str());
		exit(1);
	}

	pthread_t *threads = malloc(workers * sizeof(pthread_t));
	if (threads == NULL) {
		fatal("failed to allocate %zu bytes for threads because %s\n", workers * sizeof(pthread_t), errno_str());
		exit(1);
	}

	queue.tasks = malloc(queue_size * sizeof(task_t));
	if (queue.tasks == NULL) {
		fatal("failed to allocate %zu bytes for tasks because %s\n", queue_size * sizeof(task_t), errno_str());
		exit(1);
	}

	for (size_t index = 0; index < workers; index++) {
		if (spawn(args, threads, index) == -1) {
			exit(1);
		}
	}

	while (1) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size >= queue_size) {
			warn("waiting for queue size to decrease\n");
			pthread_cond_wait(&queue.available, &queue.lock);
		}

		if (queue.load >= workers) {
			warn("all worker threads currently busy\n");
			uint8_t new_workers = workers + 1;
			if (new_workers <= most_workers) {
				scale(&args, &threads, &workers, new_workers);
			}
		}

		pthread_mutex_unlock(&queue.lock);

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
		trace("main thread increased queue size to %zu\n", queue.size);

		pthread_cond_signal(&queue.filled);
		pthread_mutex_unlock(&queue.lock);
	}
}

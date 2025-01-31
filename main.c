#include "config.h"
#include "error.h"
#include "logger.h"
#include "thread.h"
#include <signal.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void stop(int sig) {
	signal(sig, SIG_DFL);

	trace("received signal %d\n", sig);

	pthread_mutex_lock(&thread_pool.lock);

	if (thread_pool.load > 0) {
		info("waiting for %hhu threads...\n", thread_pool.load);
	}
	while (thread_pool.load > 0) {
		trace("waiting for %hhu connections to close\n", thread_pool.load);
		pthread_cond_wait(&thread_pool.available, &thread_pool.lock);
	}

	pthread_mutex_unlock(&thread_pool.lock);

	info("graceful shutdown complete\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, &stop);
	signal(SIGTERM, &stop);

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

	if ((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fatal("failed to create socket because %s\n", errno_str());
		exit(1);
	}

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) == -1) {
		fatal("failed to set socket reuse address because %s\n", errno_str());
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

	thread_pool.workers = malloc(most_workers * sizeof(*thread_pool.workers));
	if (thread_pool.workers == NULL) {
		fatal("failed to allocate %zu bytes for workers because %s\n", most_workers * sizeof(*thread_pool.workers), errno_str());
		exit(1);
	}

	queue.tasks = malloc(queue_size * sizeof(*queue.tasks));
	if (queue.tasks == NULL) {
		fatal("failed to allocate %zu bytes for tasks because %s\n", queue_size * sizeof(*queue.tasks), errno_str());
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
				info("scaled threads from %hhu to %hhu\n", thread_pool.size, new_size);
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

		queue.tasks[queue.tail].client_sock = client_sock;
		memcpy(&queue.tasks[queue.tail].client_addr, &client_addr, sizeof(client_addr));
		queue.tail = (uint8_t)((queue.tail + 1) % (queue_size));
		queue.size++;
		trace("main thread increased queue size to %hhu\n", queue.size);

		pthread_cond_signal(&queue.filled);
		pthread_mutex_unlock(&queue.lock);
	}
}

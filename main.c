#include "config.h"
#include "error.h"
#include "logger.h"
#include "thread.h"
#include <errno.h>
#include <sqlite3.h>
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
		error("%s\n", errno_str());
		fatal("failed to create socket\n");
		exit(1);
	}

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) == -1) {
		error("%s\n", errno_str());
		fatal("failed to set socket options\n");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address);
	server_addr.sin_port = htons(port);

	if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		error("%s\n", errno_str());
		fatal("failed to bind to socket\n");
		exit(1);
	}

	if (listen(server_sock, backlog) == -1) {
		error("%s\n", errno_str());
		fatal("failed to listen on socket\n");
		exit(1);
	}

	info("listening on %s:%d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

	arg_t *args = malloc(workers * sizeof(arg_t));
	if (args == NULL) {
		error("%s\n", errno_str());
		fatal("failed to allocate for args\n");
		exit(1);
	}

	pthread_t *threads = malloc(workers * sizeof(pthread_t));
	if (threads == NULL) {
		error("%s\n", errno_str());
		fatal("failed to allocate for threads\n");
		exit(1);
	}

	queue.tasks = malloc(queue_size * sizeof(task_t));
	if (threads == NULL) {
		error("%s\n", errno_str());
		fatal("failed to allocate for tasks\n");
		exit(1);
	}

	for (size_t index = 0; index < workers; index++) {
		args[index].id = index;
		trace("spawning worker thread %zu\n", index);

		int db_error = sqlite3_open_v2(database_file, &args[index].database, SQLITE_OPEN_READWRITE, NULL);
		if (db_error != SQLITE_OK) {
			error("%s\n", sqlite3_errmsg(args[index].database));
			fatal("failed to open %s\n", database_file);
			exit(1);
		}

		int exec_error = sqlite3_exec(args[index].database, "pragma foreign_keys = on;", NULL, NULL, NULL);
		if (exec_error != SQLITE_OK) {
			error("%s\n", sqlite3_errmsg(args[index].database));
			fatal("failed to enforce foreign key constraints\n");
			exit(1);
		}

		sqlite3_busy_timeout(args[index].database, database_timeout);

		int spawn_error = pthread_create(&threads[index], NULL, thread, (void *)&args[index]);
		if (spawn_error != 0) {
			errno = spawn_error;
			error("%s\n", errno_str());
			fatal("failed to spawn worker thread %zu\n", args[index].id);
			exit(1);
		}
	}

	while (1) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size >= queue_size) {
			warn("waiting for queue size to decrease\n");
			pthread_cond_wait(&queue.available, &queue.lock);
		}

		pthread_mutex_unlock(&queue.lock);

		struct sockaddr_in client_addr;
		int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &(socklen_t){sizeof(client_addr)});

		if (client_sock == -1) {
			error("%s\n", errno_str());
			error("failed to accept client\n");
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

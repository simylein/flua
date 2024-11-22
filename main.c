#include "config.h"
#include "database.h"
#include "error.h"
#include "logger.h"
#include "thread.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int cf_errors = configure(argc, argv);
	if (cf_errors > 0) {
		fatal("config contains %d errors\n", cf_errors);
		return EXIT_FAILURE;
	}
	if (cf_errors == -1) {
		return EXIT_SUCCESS;
	}

	int db_error = sqlite3_open_v2(database_file, &database, SQLITE_OPEN_READWRITE, NULL);
	if (db_error != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		fatal("failed to open %s\n", database_file);
		return EXIT_FAILURE;
	}

	int exec_error = sqlite3_exec(database, "pragma foreign_keys = on;", NULL, NULL, NULL);
	if (exec_error != SQLITE_OK) {
		error("%s\n", sqlite3_errmsg(database));
		fatal("failed to enforce foreign key constraints\n");
		return EXIT_FAILURE;
	}

	info("using database %s\n", database_file);

	int server_sock;
	struct sockaddr_in server_addr;

	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error("%s\n", errno_str());
		fatal("failed to create socket\n");
		return EXIT_FAILURE;
	}

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) == -1) {
		error("%s\n", errno_str());
		fatal("failed to set socket options\n");
		return EXIT_FAILURE;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		error("%s\n", errno_str());
		fatal("failed to bind to socket\n");
		return EXIT_FAILURE;
	}

	if (listen(server_sock, backlog) == -1) {
		error("%s\n", errno_str());
		fatal("failed to listen on socket\n");
		return EXIT_FAILURE;
	}

	info("listening on %s:%d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

	pthread_t *threads = malloc((size_t)workers * sizeof(pthread_t));
	if (threads == NULL) {
		error("%s\n", errno_str());
		fatal("failed to allocate for threads\n");
		return EXIT_FAILURE;
	}

	queue.tasks = malloc((size_t)workers * 2 * sizeof(Task));
	if (threads == NULL) {
		error("%s\n", errno_str());
		fatal("failed to allocate for tasks\n");
		return EXIT_FAILURE;
	}

	for (int index = 0; index < workers; index++) {
		trace("spawning worker thread %d\n", index);
		pthread_create(&threads[index], NULL, thread, (void *)(intptr_t)index);
	}

	while (1) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size >= (size_t)workers * 2) {
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
		queue.back = (queue.back + 1) % ((size_t)workers * 2);
		queue.size++;
		trace("main thread increased queue size to %zu\n", queue.size);

		pthread_cond_signal(&queue.filled);
		pthread_mutex_unlock(&queue.lock);
	}
}

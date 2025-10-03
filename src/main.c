#include "api/init.h"
#include "app/page.h"
#include "lib/config.h"
#include "lib/error.h"
#include "lib/format.h"
#include "lib/logger.h"
#include "lib/thread.h"
#include <arpa/inet.h>
#include <signal.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int server_sock;
struct sockaddr_in server_addr;

void stop(int sig) {
	signal(sig, SIG_DFL);
	trace("received signal %d\n", sig);

	if (close(server_sock) == -1) {
		error("failed to close socket because %s\n", errno_str());
	}

	pthread_mutex_lock(&thread_pool.lock);

	if (thread_pool.load > 0) {
		info("waiting for %hhu threads...\n", thread_pool.load);
	}
	while (thread_pool.load > 0) {
		trace("waiting for %hhu connections to close\n", thread_pool.load);
		pthread_cond_wait(&thread_pool.available, &thread_pool.lock);
	}

	pthread_mutex_unlock(&thread_pool.lock);

	for (uint8_t index = 0; index < thread_pool.size; index++) {
		if (sqlite3_close_v2(thread_pool.workers[index].arg.database) != SQLITE_OK) {
			error("failed to close %s because %s\n", database_file, sqlite3_errmsg(thread_pool.workers[index].arg.database));
		}
		free(thread_pool.workers[index].arg.request_buffer);
		free(thread_pool.workers[index].arg.response_buffer);
	}

	free(queue.tasks);
	free(thread_pool.workers);

	page_close();
	page_free();

	info("graceful shutdown complete\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, &stop);
	signal(SIGTERM, &stop);

	logger_init();

	if (argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
		info("available command line flags\n");
		info("--name              -n   name of application              (%s)\n", name);
		info("--address           -a   ip address to bind               (%s)\n", address);
		info("--port              -p   port to listen on                (%hu)\n", port);
		info("--backlog           -b   backlog allowed on socket        (%hhu)\n", backlog);
		info("--queue-size        -qs  size of clients in queue         (%hhu)\n", queue_size);
		info("--least-workers     -lw  least amount of worker threads   (%hhu)\n", least_workers);
		info("--most-workers      -mw  most amount of worker threads    (%hhu)\n", most_workers);
		info("--bwt-key           -bk  random bytes for bwt signing     (%s)\n", bwt_key);
		info("--bwt-ttl           -bt  time to live for bwt expiry      (%u)\n", bwt_ttl);
		info("--database-file     -df  path to sqlite database file     (%s)\n", database_file);
		info("--database-timeout  -dt  milliseconds to wait for lock    (%hu)\n", database_timeout);
		info("--receive-timeout   -rt  seconds to wait for receiving    (%hhu)\n", receive_timeout);
		info("--send-timeout      -st  seconds to wait for sending      (%hhu)\n", send_timeout);
		info("--receive-packets   -rp  most packets allowed to receive  (%hhu)\n", receive_packets);
		info("--send-packets      -sp  most packets allowed to send     (%hhu)\n", send_packets);
		info("--receive-buffer    -rb  most bytes in receive buffer     (%u)\n", receive_buffer);
		info("--send-buffer       -sb  most bytes in send buffer        (%u)\n", send_buffer);
		info("--log-level         -ll  logging verbosity to print       (%s)\n", human_log_level(log_level));
		info("--log-requests      -lq  log incoming requests            (%s)\n", human_bool(log_requests));
		info("--log-responses     -ls  log outgoing responses           (%s)\n", human_bool(log_responses));
		exit(0);
	}

	if (argc >= 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)) {
		info("flua version %s %s\n", version, commit);
		info("written by simylein in c\n");
		exit(0);
	}

	uint8_t cmds = 0x00;
	int cf_errors = configure(argc, argv, &cmds);
	if (cf_errors != 0) {
		fatal("config contains %d errors\n", cf_errors);
		exit(1);
	}

	if (cmds != 0x00) {
		sqlite3 *database;
		if (sqlite3_open_v2(database_file, &database, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) {
			fatal("failed to open %s because %s\n", database_file, sqlite3_errmsg(database));
			exit(1);
		}

		if (cmds & 0x10 && init(database) != 0) {
			fatal("failed to initialise database\n");
			exit(1);
		}

		if (sqlite3_close_v2(database) != SQLITE_OK) {
			fatal("failed to close %s because %s\n", database_file, sqlite3_errmsg(database));
			exit(1);
		}

		exit(0);
	}

	page_init();

	info("starting flua application\n");

	queue.tasks = malloc(queue_size * sizeof(*queue.tasks));
	if (queue.tasks == NULL) {
		fatal("failed to allocate %zu bytes for tasks because %s\n", queue_size * sizeof(*queue.tasks), errno_str());
		exit(1);
	}

	thread_pool.workers = malloc(most_workers * sizeof(*thread_pool.workers));
	if (thread_pool.workers == NULL) {
		fatal("failed to allocate %zu bytes for workers because %s\n", most_workers * sizeof(*thread_pool.workers), errno_str());
		exit(1);
	}

	for (uint8_t index = 0; index < least_workers; index++) {
		if (spawn(&thread_pool.workers[index], thread_pool.size, &thread, &fatal) == -1) {
			exit(1);
		}
		thread_pool.size++;
	}

	info("spawned %hhu worker threads\n", least_workers);

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

	info("listening on %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

	while (true) {
		pthread_mutex_lock(&queue.lock);

		while (queue.size >= queue_size) {
			warn("waiting for queue size %hhu to decrease\n", queue.size);
			pthread_cond_wait(&queue.available, &queue.lock);
		}

		pthread_mutex_unlock(&queue.lock);

		pthread_mutex_lock(&thread_pool.lock);

		if (thread_pool.load >= thread_pool.size) {
			debug("all worker threads currently busy\n");
			uint8_t new_size = thread_pool.size + 1;
			if (new_size <= most_workers && spawn(&thread_pool.workers[thread_pool.size], thread_pool.size, &thread, &error) == 0) {
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
		queue.tail = (uint8_t)((queue.tail + 1) % queue_size);
		queue.size++;
		trace("main thread increased queue size to %hhu\n", queue.size);

		pthread_cond_signal(&queue.filled);
		pthread_mutex_unlock(&queue.lock);
	}
}

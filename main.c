#include "app.h"
#include "config.h"
#include "database.h"
#include "error.h"
#include "format.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
	if (db_error != 0) {
		error("%s\n", sqlite3_errmsg(database));
		fatal("failed to open %s\n", database_file);
		return EXIT_FAILURE;
	}

	info("using database %s\n", database_file);

	int server_sock;
	struct sockaddr_in server_addr;

	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		error("%s\n", errno_str());
		fatal("failed to create socket\n");
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

	while (1) {
		struct sockaddr_in client_addr;
		int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &(socklen_t){sizeof(client_addr)});

		if (client_sock == -1) {
			error("%s\n", errno_str());
			error("failed to accept client\n");
			continue;
		}

		char request_buffer[12288] = {0};
		ssize_t bytes_received = recv(client_sock, request_buffer, sizeof(request_buffer), 0);

		if (bytes_received == -1) {
			error("%s\n", errno_str());
			error("failed to receive data from client\n");
			goto cleanup;
		}
		if (bytes_received == 0) {
			warn("client did not send any data\n");
			goto cleanup;
		}

		const char *length_name = "content-length:";
		char *length_index = strcasestr(request_buffer, length_name);
		char *body_index = strcasestr(request_buffer, "\r\n\r\n");
		if (length_index && body_index) {
			length_index += strlen(length_name);
			size_t content_length = (size_t)atoi(length_index);
			size_t body_length = (size_t)bytes_received - (size_t)(body_index - request_buffer + 4);

			while (body_length < content_length) {
				ssize_t more_bytes =
						recv(client_sock, &request_buffer[bytes_received], sizeof(request_buffer) - (size_t)bytes_received, 0);

				if (more_bytes == -1) {
					error("%s\n", errno_str());
					error("failed to receive more data from client\n");
					goto cleanup;
				}
				if (more_bytes == 0) {
					warn("client did not send complete data\n");
					goto cleanup;
				}

				body_length += (size_t)more_bytes;
				bytes_received += more_bytes;
			}
		}

		trace("received %zd bytes from %s:%d\n", bytes_received, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		struct timespec start;
		clock_gettime(CLOCK_MONOTONIC, &start);

		struct Request reqs = {
				.method = {0}, .pathname = {0}, .search = {0}, .protocol = {0}, .header = {0}, .body = {0}, .body_len = 0};
		struct Response resp = {.status = 0, .header = {0}, .body = {0}, .body_len = 0};

		request(&request_buffer, bytes_received, &reqs, &resp);
		req("%s %s %s\n", reqs.method, reqs.pathname, human_bytes((size_t)bytes_received));

		if (resp.status == 0) {
			handle(&reqs, &resp);
		}

		char response_buffer[12288] = {0};
		size_t response_length = response(&response_buffer, &resp);

		struct timespec stop;
		clock_gettime(CLOCK_MONOTONIC, &stop);

		res("%d %s %s\n", resp.status, human_duration(&start, &stop), human_bytes(response_length));
		ssize_t bytes_sent = send(client_sock, response_buffer, response_length, 0);

		trace("sent %zd bytes to %s:%d\n", bytes_sent, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		if (bytes_sent == -1) {
			error("%s\n", errno_str());
			error("failed to send data to client\n");
		}

	cleanup:
		if (close(client_sock) == -1) {
			error("%s\n", errno_str());
			error("failed to close client socket\n");
		}
	}
}

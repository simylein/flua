#include "error.h"
#include "format.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "router.h"
#include <unistd.h>

void null_init(Request *req, Response *res) {
	req->method[0] = '\0';
	req->method_len = 0;
	req->pathname[0] = '\0';
	req->pathname_len = 0;
	req->search[0] = '\0';
	req->search_len = 0;
	req->protocol[0] = '\0';
	req->protocol_len = 0;
	req->header[0] = '\0';
	req->header_len = 0;
	req->body_len = 0;

	res->status = 0;
	res->head_len = 0;
	res->header[0] = '\0';
	res->header_len = 0;
	res->body_len = 0;
}

void handle(int *client_sock, struct sockaddr_in *client_addr) {
	char request_buffer[20480];
	ssize_t bytes_received = recv(*client_sock, request_buffer, sizeof(request_buffer), 0);

	if (bytes_received == -1) {
		error("%s\n", errno_str());
		error("failed to receive data from client\n");
		goto cleanup;
	}
	if (bytes_received == 0) {
		warn("client did not send any data\n");
		goto cleanup;
	}

	trace("received %zd bytes from %s:%d\n", bytes_received, inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));

	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	struct Request reqs;
	struct Response resp;
	null_init(&reqs, &resp);

	char bytes_buffer[8];
	human_bytes(&bytes_buffer, (size_t)bytes_received);

	request(&request_buffer, bytes_received, &reqs, &resp);
	trace("method %zub pathname %zub search %zub header %zub body %zub\n", reqs.method_len, reqs.pathname_len, reqs.search_len,
				reqs.header_len, reqs.body_len);
	req("%s %s %s\n", reqs.method, reqs.pathname, bytes_buffer);

	if (resp.status == 0) {
		route(&reqs, &resp);
	}

	char response_buffer[20480];
	size_t response_length = response(&response_buffer, &resp);

	struct timespec stop;
	clock_gettime(CLOCK_MONOTONIC, &stop);

	char duration_buffer[8];
	human_duration(&duration_buffer, &start, &stop);
	human_bytes(&bytes_buffer, response_length);

	res("%d %s %s\n", resp.status, duration_buffer, bytes_buffer);
	trace("head %zub header %zub body %zub\n", resp.head_len, resp.header_len, resp.body_len);
	ssize_t bytes_sent = send(*client_sock, response_buffer, response_length, 0);

	if (bytes_sent == -1) {
		error("%s\n", errno_str());
		error("failed to send data to client\n");
		goto cleanup;
	}
	if (bytes_sent == 0) {
		warn("server did not send any data\n");
		goto cleanup;
	}

	trace("sent %zd bytes to %s:%d\n", bytes_sent, inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));

cleanup:
	if (close(*client_sock) == -1) {
		error("%s\n", errno_str());
		error("failed to close client socket\n");
	}
}

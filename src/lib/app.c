#include "../api/router.h"
#include "config.h"
#include "error.h"
#include "format.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "strn.h"
#include <arpa/inet.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void handle(sqlite3 *database, char *request_buffer, char *response_buffer, int *client_sock, struct sockaddr_in *client_addr) {
	struct request_t reqs;
	struct response_t resp;

	request_init(&reqs);
	response_init(&resp, response_buffer);

	if (setsockopt(*client_sock, SOL_SOCKET, SO_RCVTIMEO, &(struct timeval){.tv_sec = receive_timeout, .tv_usec = 0},
								 sizeof(struct timeval)) == -1) {
		error("failed to set socket receive timeout because %s\n", errno_str());
		goto cleanup;
	}

	if (setsockopt(*client_sock, SOL_SOCKET, SO_SNDTIMEO, &(struct timeval){.tv_sec = send_timeout, .tv_usec = 0},
								 sizeof(struct timeval)) == -1) {
		error("failed to set socket send timeout because %s\n", errno_str());
		goto cleanup;
	}

	size_t request_length = 0;

	size_t received_bytes = 0;
	uint8_t received_packets = 0;
	ssize_t received =
			recv(*client_sock, request_buffer,
					 (size_t)(reqs.method.cap + reqs.pathname.cap + reqs.search.cap + reqs.protocol.cap + reqs.header.cap), 0);

	if (received == -1) {
		error("failed to receive data from client because %s\n", errno_str());
		goto cleanup;
	}
	if (received == 0) {
		warn("client did not send any data\n");
		goto cleanup;
	}

	received_bytes += (size_t)received;
	received_packets++;

	const char *length_index = strncasestrn(request_buffer, received_bytes, "content-length:", 15);
	const char *body_index = strncasestrn(request_buffer, received_bytes, "\r\n\r\n", 4);

	if (length_index && body_index) {
		length_index += 15;
		body_index += 4;

		const size_t content_length = (size_t)atoi(length_index);
		if (content_length > 0) {
			request_length = (size_t)(body_index - request_buffer) + content_length;
		}
	}

	while (received_bytes < request_length) {
		if (received_packets + 1 > receive_packets) {
			warn("packets received %hhu exceeds allowed packets %hhu\n", received_packets, receive_packets);
			break;
		}

		if (request_length > receive_buffer) {
			warn("request length %zu exceeds buffer length %u\n", request_length, receive_buffer);
			break;
		}

		ssize_t received_further = recv(*client_sock, &request_buffer[received_bytes], receive_buffer - received_bytes, 0);

		if (received_further == -1) {
			error("failed to receive further data from client because %s\n", errno_str());
			goto cleanup;
		}
		if (received_further == 0) {
			warn("client did not send any further data\n");
			goto cleanup;
		}

		received_bytes += (size_t)received_further;
		received_packets++;
	}

	trace("received %zu bytes in %hhu packets from %s:%d\n", received_bytes, received_packets, inet_ntoa(client_addr->sin_addr),
				ntohs(client_addr->sin_port));

	if (shutdown(*client_sock, SHUT_RD) == -1) {
		error("failed to shutdown client socket reading because %s\n", errno_str());
	}

	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	char bytes_buffer[8];
	human_bytes(&bytes_buffer, received_bytes);

	request(request_buffer, received_bytes, &reqs, &resp);
	trace("method %hhub pathname %hhub search %hub header %hub body %ub\n", reqs.method.len, reqs.pathname.len, reqs.search.len,
				reqs.header.len, reqs.body.len);
	req("%.*s %.*s %s\n", (int)reqs.method.len, reqs.method.ptr, (int)reqs.pathname.len, reqs.pathname.ptr, bytes_buffer);

	route(database, &reqs, &resp);

	size_t response_length = response(&reqs, &resp, response_buffer);

	struct timespec stop;
	clock_gettime(CLOCK_MONOTONIC, &stop);

	char duration_buffer[8];
	human_duration(&duration_buffer, &start, &stop);
	human_bytes(&bytes_buffer, response_length);

	res("%d %s %s\n", resp.status, duration_buffer, bytes_buffer);
	trace("head %hhub header %hub body %ub\n", resp.head.len, resp.header.len, resp.body.len);

	size_t sent_bytes = 0;
	uint8_t sent_packets = 0;
	ssize_t sent = send(*client_sock, response_buffer, resp.head.len + resp.header.len, MSG_NOSIGNAL);

	if (sent == -1) {
		error("failed to send data to client because %s\n", errno_str());
		goto cleanup;
	}
	if (sent == 0) {
		warn("server did not send any data\n");
		goto cleanup;
	}

	sent_bytes += (size_t)sent;
	sent_packets++;

	while ((size_t)sent_bytes < response_length) {
		if (sent_packets + 1 > send_packets) {
			warn("packets sent %hhu exceeds allowed packets %hhu\n\n", sent_packets, send_packets);
			break;
		}

		if (response_length > send_buffer) {
			warn("response length %zu exceeds buffer length %u\n", response_length, send_buffer);
			break;
		}

		ssize_t sent_further = send(*client_sock, &resp.body.ptr[sent_bytes - resp.head.len - resp.header.len],
																resp.body.len - (sent_bytes - resp.head.len - resp.header.len), MSG_NOSIGNAL);

		if (sent_further == -1) {
			error("failed to send further data to client because %s\n", errno_str());
			goto cleanup;
		}
		if (sent_further == 0) {
			warn("server did not send any further data\n");
			goto cleanup;
		}

		sent_bytes += (size_t)sent_further;
		sent_packets++;
	}

	trace("sent %zu bytes in %hhu packets to %s:%d\n", sent_bytes, sent_packets, inet_ntoa(client_addr->sin_addr),
				ntohs(client_addr->sin_port));

	if (shutdown(*client_sock, SHUT_WR) == -1) {
		error("failed to shutdown client socket writing because %s\n", errno_str());
	}

cleanup:
	if (close(*client_sock) == -1) {
		error("failed to close client socket because %s\n", errno_str());
	}
}

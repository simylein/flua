#include "endian.h"
#include "flight.h"
#include "logger.h"
#include "request.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

int parse_credentials(char (*username)[17], char (*password)[65], request_t *request) {
	int stage = 0;
	size_t index = 0;

	size_t username_length = 0;
	const size_t username_index = index;
	while (stage == 0 && username_length < sizeof(*username) - 1 && index < request->body_len) {
		char *byte = &request->body[index];
		if (*byte == '\0') {
			stage = 1;
		} else {
			username_length++;
		}
		index++;
	}
	memcpy(username, &request->body[username_index], username_length);
	(*username)[username_length] = '\0';
	if (stage != 1) {
		return -1;
	}

	size_t password_length = 0;
	const size_t password_index = index;
	while (stage == 1 && password_length < sizeof(*password) - 1 && index < request->body_len) {
		char *byte = &request->body[index];
		if (*byte == '\0') {
			stage = 2;
		} else {
			password_length++;
		}
		index++;
	}
	memcpy(password, &request->body[password_index], password_length);
	(*password)[password_length] = '\0';
	if (stage != 2) {
		return -1;
	}

	trace("username %zu bytes and password %zu bytes\n", username_length, password_length);
	return 0;
}

int validate_credentials(char (*username)[17], char (*password)[65]) {
	size_t username_index = 0;
	const size_t username_length = strlen(*username);
	if (username_length < 4) {
		return -1;
	}

	while (username_index < username_length) {
		char *byte = &(*username)[username_index];
		if (*byte < 'a' || *byte > 'z') {
			return -1;
		}
		username_index++;
	}

	size_t password_index = 0;
	const size_t password_length = strlen(*password);
	if (password_length < 4) {
		return -1;
	}

	bool lower = false;
	bool upper = false;
	bool digit = false;

	while (password_index < password_length) {
		char *byte = &(*password)[password_index];
		if (*byte >= '0' && *byte <= '9') {
			digit = true;
		} else if (*byte >= 'a' && *byte <= 'z') {
			lower = true;
		} else if (*byte >= 'A' && *byte <= 'Z') {
			upper = true;
		}
		password_index++;
	}

	if (!lower || !upper || !digit) {
		return -1;
	}

	return 0;
}

int parse_flight(flight_t *flight, request_t *request) {
	if (request->body_len != 48) {
		return -1;
	}

	uint64_t n_starts_at;
	uint64_t n_ends_at;

	memcpy(&flight->hash, request->body, sizeof(flight->hash));
	memcpy(&n_starts_at, &request->body[sizeof(flight->hash)], sizeof(flight->starts_at));
	memcpy(&n_ends_at, &request->body[sizeof(flight->hash) + sizeof(flight->starts_at)], sizeof(flight->ends_at));

	flight->starts_at = ntohll(n_starts_at);
	flight->ends_at = ntohll(n_ends_at);

	return 0;
}

int validate_flight(flight_t *flight) {
	if (flight->starts_at > flight->ends_at) {
		return -1;
	}

	return 0;
}

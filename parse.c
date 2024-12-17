#include "endian.h"
#include "flight.h"
#include "logger.h"
#include "request.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

int parse_credentials(char **username, uint8_t *username_len, char **password, uint8_t *password_len, request_t *request) {
	int stage = 0;
	uint8_t index = 0;

	*username_len = 0;
	const uint8_t username_index = index;
	while (stage == 0 && *username_len < 16 && index < request->body_len) {
		char *byte = &request->body[index];
		if (*byte == '\0') {
			stage = 1;
		} else {
			(*username_len)++;
		}
		index++;
	}
	*username = &request->body[username_index];
	if (stage != 1) {
		return -1;
	}

	*password_len = 0;
	const uint8_t password_index = index;
	while (stage == 1 && *password_len < 64 && index < request->body_len) {
		char *byte = &request->body[index];
		if (*byte == '\0') {
			stage = 2;
		} else {
			(*password_len)++;
		}
		index++;
	}
	*password = &request->body[password_index];
	if (stage != 2) {
		return -1;
	}

	trace("username %hhu bytes and password %hhu bytes\n", *username_len, *password_len);
	return 0;
}

int validate_credentials(char **username, uint8_t *username_len, char **password, uint8_t *password_len) {
	if (*username_len < 4) {
		return -1;
	}

	uint8_t username_index = 0;
	while (username_index < *username_len) {
		char *byte = &(*username)[username_index];
		if (*byte < 'a' || *byte > 'z') {
			return -1;
		}
		username_index++;
	}

	if (*password_len < 4) {
		return -1;
	}

	bool lower = false;
	bool upper = false;
	bool digit = false;

	uint8_t password_index = 0;
	while (password_index < *password_len) {
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
	if (request->body_len != 68) {
		return -1;
	}

	uint64_t n_starts_at;
	uint64_t n_ends_at;

	memcpy(&flight->hash, request->body, sizeof(flight->hash));

	size_t starts_at_offset = sizeof(flight->hash);
	memcpy(&n_starts_at, &request->body[starts_at_offset], sizeof(flight->starts_at));

	size_t ends_at_offset = starts_at_offset + sizeof(flight->starts_at);
	memcpy(&n_ends_at, &request->body[ends_at_offset], sizeof(flight->ends_at));

	size_t altitude_offset = ends_at_offset + sizeof(flight->ends_at);
	memcpy(&flight->altitude, &request->body[altitude_offset], sizeof(flight->altitude));

	size_t thermal_offset = altitude_offset + sizeof(flight->altitude);
	memcpy(&flight->thermal, &request->body[thermal_offset], sizeof(flight->thermal));

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

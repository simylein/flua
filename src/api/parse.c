#include "../lib/endian.h"
#include "../lib/logger.h"
#include "../lib/request.h"
#include "flight.h"
#include "user.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

int parse_credentials(char **username, uint8_t *username_len, char **password, uint8_t *password_len, request_t *request) {
	int stage = 0;
	uint8_t index = 0;

	*username_len = 0;
	const uint8_t username_index = index;
	while (stage == 0 && *username_len < 16 && index < request->body.len) {
		char *byte = &request->body.ptr[index];
		if (*byte == '\0') {
			stage = 1;
		} else {
			(*username_len)++;
		}
		index++;
	}
	*username = &request->body.ptr[username_index];
	if (stage != 1) {
		return -1;
	}

	*password_len = 0;
	const uint8_t password_index = index;
	while (stage == 1 && *password_len < 64 && index < request->body.len) {
		char *byte = &request->body.ptr[index];
		if (*byte == '\0') {
			stage = 2;
		} else {
			(*password_len)++;
		}
		index++;
	}
	*password = &request->body.ptr[password_index];
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

	if ((*username_len == 4 && memcmp(*username, "demo", 4) == 0) ||
			(*username_len == 6 && (memcmp(*username, "signin", 6) == 0 || memcmp(*username, "signup", 6) == 0)) ||
			(*username_len == 8 && memcmp(*username, "settings", 8) == 0)) {
		return -1;
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

int parse_user(user_t *user, request_t *request) {
	if (request->body.len != sizeof(user->visibility)) {
		return -1;
	}

	uint8_t offset = 0;

	user->visibility = (uint8_t)(request->body.ptr[offset]);
	offset += sizeof(user->visibility);

	return 0;
}

int validate_user(user_t *user) {
	if (user->visibility != private && user->visibility != friends && user->visibility != public) {
		return -1;
	}

	return 0;
}

int parse_flight(flight_t *flight, request_t *request) {
	if (request->body.len != sizeof(*flight->hash) + sizeof(*flight->starts_at) + sizeof(*flight->ends_at) +
															 sizeof(*flight->altitude_bins) + sizeof(*flight->altitude_min) + sizeof(*flight->altitude_max) +
															 sizeof(*flight->thermal_bins) + sizeof(*flight->max_climb) + sizeof(*flight->max_sink) +
															 sizeof(*flight->speed_bins) + sizeof(*flight->speed_avg) + sizeof(*flight->speed_max) +
															 sizeof(*flight->glide_bins) + sizeof(*flight->distance_flown)) {
		return -1;
	}

	uint8_t offset = 0;

	flight->hash = (uint8_t (*)[32])(&request->body.ptr[offset]);
	offset += sizeof(*flight->hash);

	(*flight).starts_at = (uint64_t(*))(&request->body.ptr[offset]);
	*(*flight).starts_at = ntoh64(*(*flight).starts_at);
	offset += sizeof(*flight->starts_at);

	(*flight).ends_at = (uint64_t(*))(&request->body.ptr[offset]);
	*(*flight).ends_at = ntoh64(*(*flight).ends_at);
	offset += sizeof(*flight->ends_at);

	(*flight).altitude_bins = (uint16_t (*)[5])(&request->body.ptr[offset]);
	offset += sizeof(*flight->altitude_bins);

	(*flight).altitude_min = (uint16_t *)(&request->body.ptr[offset]);
	*(*flight).altitude_min = ntoh16(*(*flight).altitude_min);
	offset += sizeof(*flight->altitude_min);

	(*flight).altitude_max = (uint16_t *)(&request->body.ptr[offset]);
	*(*flight).altitude_max = ntoh16(*(*flight).altitude_max);
	offset += sizeof(*flight->altitude_max);

	(*flight).thermal_bins = (uint16_t (*)[5])(&request->body.ptr[offset]);
	offset += sizeof(*flight->thermal_bins);

	(*flight).max_climb = (uint8_t *)(&request->body.ptr[offset]);
	offset += sizeof(*flight->max_climb);

	(*flight).max_sink = (uint8_t *)(&request->body.ptr[offset]);
	offset += sizeof(*flight->max_sink);

	(*flight).speed_bins = (uint16_t (*)[5])(&request->body.ptr[offset]);
	offset += sizeof(*flight->speed_bins);

	(*flight).speed_avg = (uint16_t *)(&request->body.ptr[offset]);
	*(*flight).speed_avg = ntoh16(*(*flight).speed_avg);
	offset += sizeof(*flight->speed_avg);

	(*flight).speed_max = (uint16_t *)(&request->body.ptr[offset]);
	*(*flight).speed_max = ntoh16(*(*flight).speed_max);
	offset += sizeof(*flight->speed_max);

	(*flight).glide_bins = (uint16_t (*)[5])(&request->body.ptr[offset]);
	offset += sizeof(*flight->glide_bins);

	(*flight).distance_flown = (uint32_t *)(&request->body.ptr[offset]);
	*(*flight).distance_flown = ntoh32(*(*flight).distance_flown);

	return 0;
}

int validate_flight(flight_t *flight) {
	if (*flight->starts_at == 0 || *flight->ends_at == 0) {
		return -1;
	}

	if (*flight->starts_at > *flight->ends_at) {
		return -1;
	}

	if (*flight->ends_at - *flight->starts_at < 64 || *flight->ends_at - *flight->starts_at > 65536) {
		return -1;
	}

	if (*flight->altitude_min > *flight->altitude_max) {
		return -1;
	}

	if (*flight->speed_avg > *flight->speed_max) {
		return -1;
	}

	return 0;
}

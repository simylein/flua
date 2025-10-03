#include "logger.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const char *name = "flua";

const char *address = "0.0.0.0";
uint16_t port = 2254;

uint8_t backlog = 16;
uint8_t queue_size = 8;
uint8_t least_workers = 4;
uint8_t most_workers = 64;

const char *bwt_key = "f66l6cu75a61";
uint32_t bwt_ttl = 2764800;

const char *database_file = "flua.sqlite";
uint16_t database_timeout = 500;

uint8_t receive_timeout = 60;
uint8_t send_timeout = 60;
uint8_t receive_packets = 16;
uint8_t send_packets = 16;
uint32_t receive_buffer = 262144;
uint32_t send_buffer = 262144;

uint8_t log_level = 4;
bool log_requests = true;
bool log_responses = true;

bool match_arg(const char *flag, const char *verbose, const char *concise) {
	return strcmp(flag, verbose) == 0 || strcmp(flag, concise) == 0;
}

const char *next_arg(const int argc, char *argv[], int *ind) {
	(*ind)++;
	if (*ind < argc) {
		return argv[*ind];
	}
	return NULL;
}

int parse_bool(const char *arg, const char *key, bool *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", key);
		return 1;
	}

	if (strcmp(arg, "false") == 0) {
		*value = false;
	} else if (strcmp(arg, "true") == 0) {
		*value = true;
	} else {
		error("%s must be either true or false\n", key);
		return 1;
	}

	return 0;
}

int parse_uint8(const char *arg, const char *key, const uint8_t min, const uint8_t max, uint8_t *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", key);
		return 1;
	}

	char *arg_end;
	const uint64_t new_value = strtoul(arg, &arg_end, 10);
	if (*arg_end != '\0') {
		error("%s must be an unsigned integer\n", key);
		return 1;
	}

	if (new_value < min || new_value > max) {
		error("%s must be between %hhu and %hhu\n", key, min, max);
		return 1;
	}

	*value = (uint8_t)new_value;
	return 0;
}

int parse_uint16(const char *arg, const char *key, const uint16_t min, const uint16_t max, uint16_t *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", key);
		return 1;
	}

	char *arg_end;
	const uint64_t new_value = strtoul(arg, &arg_end, 10);
	if (*arg_end != '\0') {
		error("%s must be an unsigned integer\n", key);
		return 1;
	}

	if (new_value < min || new_value > max) {
		error("%s must be between %hu and %hu\n", key, min, max);
		return 1;
	}

	*value = (uint16_t)new_value;
	return 0;
}

int parse_uint32(const char *arg, const char *key, const uint32_t min, const uint32_t max, uint32_t *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", key);
		return 1;
	}

	char *arg_end;
	const uint64_t new_value = strtoul(arg, &arg_end, 10);
	if (*arg_end != '\0') {
		error("%s must be an unsigned integer\n", key);
		return 1;
	}

	if (new_value < min || new_value > max) {
		error("%s must be between %u and %u\n", key, min, max);
		return 1;
	}

	*value = (uint32_t)new_value;
	return 0;
}

int parse_str(const char *arg, const char *key, size_t min, size_t max, const char **value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", key);
		return 1;
	}

	size_t len = strlen(arg);
	if (len < min || len > max) {
		error("%s must be between %zu and %zu characters\n", key, min, max);
		return 1;
	}

	*value = arg;
	return 0;
}

int parse_log_level(const char *arg, uint8_t *value) {
	if (arg == NULL) {
		error("please provide a value for log level\n");
		return 1;
	}

	if (strcmp(arg, "fatal") == 0) {
		*value = 1;
	} else if (strcmp(arg, "error") == 0) {
		*value = 2;
	} else if (strcmp(arg, "warn") == 0) {
		*value = 3;
	} else if (strcmp(arg, "info") == 0) {
		*value = 4;
	} else if (strcmp(arg, "debug") == 0) {
		*value = 5;
	} else if (strcmp(arg, "trace") == 0) {
		*value = 6;
	} else {
		error("log level must be one of trace debug info warn error fatal\n");
		return 1;
	}

	return 0;
}

int configure(int argc, char *argv[], uint8_t *cmds) {
	int errors = 0;

	for (int ind = 1; ind < argc; ind++) {
		const char *flag = argv[ind];
		if (match_arg(flag, "--init", "-i")) {
			*cmds |= 0x10;
		} else if (match_arg(flag, "--seed", "-s")) {
			*cmds |= 0x20;
		} else if (match_arg(flag, "--name", "-n")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_str(value, "name", 2, 8, &name);
		} else if (match_arg(flag, "--address", "-a")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_str(value, "address", 4, 16, &address);
		} else if (match_arg(flag, "--port", "-p")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint16(value, "port", 0, 65535, &port);
		} else if (match_arg(flag, "--backlog", "-b")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "backlog", 0, 255, &backlog);
		} else if (match_arg(flag, "--queue-size", "-qs")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "queue size", 1, 127, &queue_size);
		} else if (match_arg(flag, "--least-workers", "-lw")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "least-workers", 1, 63, &least_workers);
		} else if (match_arg(flag, "--most-workers", "-mw")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "most-workers", 3, 255, &most_workers);
		} else if (match_arg(flag, "--bwt-key", "-bk")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_str(value, "bwt key", 16, 64, &bwt_key);
		} else if (match_arg(flag, "--bwt-ttl", "-bt")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint32(value, "bwt ttl", 3600, 15768000, &bwt_ttl);
		} else if (match_arg(flag, "--database-file", "-df")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_str(value, "database file", 4, 64, &database_file);
		} else if (match_arg(flag, "--database-timeout", "-dt")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint16(value, "database timeout", 10, 10000, &database_timeout);
		} else if (match_arg(flag, "--receive-timeout", "-rt")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "receive timeout", 2, 240, &receive_timeout);
		} else if (match_arg(flag, "--send-timeout", "-st")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "send timeout", 2, 240, &send_timeout);
		} else if (match_arg(flag, "--receive-packets", "-rp")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "receive packets", 1, 128, &receive_packets);
		} else if (match_arg(flag, "--send-packets", "-sp")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint8(value, "send packets", 1, 128, &send_packets);
		} else if (match_arg(flag, "--receive-buffer", "-rb")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint32(value, "receive buffer", 16384, 1048576, &receive_buffer);
		} else if (match_arg(flag, "--send-buffer", "-sb")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_uint32(value, "send buffer", 16384, 1048576, &send_buffer);
		} else if (match_arg(flag, "--log-level", "-ll")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_log_level(value, &log_level);
		} else if (match_arg(flag, "--log-requests", "-lq")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_bool(value, "log requests", &log_requests);
		} else if (match_arg(flag, "--log-responses", "-ls")) {
			const char *value = next_arg(argc, argv, &ind);
			errors += parse_bool(value, "log responses", &log_responses);
		} else {
			errors++;
			error("unknown argument %s\n", flag);
		}
	}

	return errors;
}

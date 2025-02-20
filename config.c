#include "logger.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const char *address = "0.0.0.0";
uint16_t port = 2254;

uint8_t backlog = 16;
uint8_t queue_size = 8;
uint8_t least_workers = 4;
uint8_t most_workers = 64;

uint32_t bwt_ttl = 2764800;
const char *bwt_key = "f2l2u5a4";

const char *database_file = "flua.sqlite";
uint16_t database_timeout = 500;

uint8_t receive_timeout = 60;
uint8_t send_timeout = 60;

uint8_t log_level = 4;
bool log_requests = true;
bool log_responses = true;

const char *next_arg(const int argc, char *argv[], int *ind) {
	(*ind)++;
	if (*ind < argc) {
		return argv[*ind];
	}
	return NULL;
}

int parse_uint8(const char *arg, const char *name, const uint8_t min, const uint8_t max, uint8_t *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", name);
		return 1;
	}

	char *arg_end;
	const uint64_t new_value = strtoul(arg, &arg_end, 10);
	if (*arg_end != '\0') {
		error("%s must be an unsigned integer\n", name);
		return 1;
	}

	if (new_value < min || new_value > max) {
		error("%s must be between %hhu and %hhu\n", name, min, max);
		return 1;
	}

	*value = (uint8_t)new_value;
	return 0;
}

int parse_uint16(const char *arg, const char *name, const uint16_t min, const uint16_t max, uint16_t *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", name);
		return 1;
	}

	char *arg_end;
	const uint64_t new_value = strtoul(arg, &arg_end, 10);
	if (*arg_end != '\0') {
		error("%s must be an unsigned integer\n", name);
		return 1;
	}

	if (new_value < min || new_value > max) {
		error("%s must be between %hu and %hu\n", name, min, max);
		return 1;
	}

	*value = (uint16_t)new_value;
	return 0;
}

int parse_uint32(const char *arg, const char *name, const uint32_t min, const uint32_t max, uint32_t *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", name);
		return 1;
	}

	char *arg_end;
	const uint64_t new_value = strtoul(arg, &arg_end, 10);
	if (*arg_end != '\0') {
		error("%s must be an unsigned integer\n", name);
		return 1;
	}

	if (new_value < min || new_value > max) {
		error("%s must be between %u and %u\n", name, min, max);
		return 1;
	}

	*value = (uint32_t)new_value;
	return 0;
}

int parse_bool(const char *arg, const char *name, bool *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", name);
		return 1;
	}

	if (strcmp(arg, "false") == 0) {
		*value = false;
	} else if (strcmp(arg, "true") == 0) {
		*value = true;
	} else {
		error("%s must be either true or false\n", name);
		return 1;
	}

	return 0;
}

int parse_str(const char *arg, const char *name, size_t min, size_t max, const char **value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", name);
		return 1;
	}

	size_t len = strlen(arg);
	if (len < min || len > max) {
		error("%s must be between %zu and %zu characters\n", name, min, max);
		return 1;
	}

	*value = arg;
	return 0;
}

int parse_log_level(const char *arg, const char *name, uint8_t *value) {
	if (arg == NULL) {
		error("please provide a value for %s\n", name);
		return 1;
	}

	if (strcmp(arg, "panic") == 0) {
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
		error("%s must be one of trace debug info warn error panic\n", name);
		return 1;
	}

	return 0;
}

const char *human_bool(int val) {
	switch (val) {
	case 0:
		return "false";
	case 1:
		return "true";
	default:
		return "???";
	}
}

const char *human_log_level(int level) {
	switch (level) {
	case 1:
		return "panic";
	case 2:
		return "error";
	case 3:
		return "warn";
	case 4:
		return "info";
	case 5:
		return "debug";
	case 6:
		return "trace";
	default:
		return "???";
	}
}

int configure(int argc, char *argv[]) {
	int errors = 0;

	for (int ind = 1; ind < argc; ind++) {
		const char *flag = argv[ind];
		if (strcmp(flag, "--help") == 0 || strcmp(flag, "-h") == 0) {
			info("available command line flags\n");
			info("--address           -a   ip address to bind              (%s)\n", address);
			info("--port              -p   port to listen on               (%hu)\n", port);
			info("--backlog           -b   backlog allowed on socket       (%hhu)\n", backlog);
			info("--queue-size        -qs  size of clients in queue        (%hhu)\n", queue_size);
			info("--least-workers     -lw  least amount of worker threads  (%hhu)\n", least_workers);
			info("--most-workers      -mw  most amount of worker threads   (%hhu)\n", most_workers);
			info("--bwt-ttl           -bt  time to live for bwt expiry     (%u)\n", bwt_ttl);
			info("--bwt-key           -bk  random bytes for bwt signing    (%s)\n", bwt_key);
			info("--database-file     -df  path to sqlite database file    (%s)\n", database_file);
			info("--database-timeout  -dt  milliseconds to wait for lock   (%hu)\n", database_timeout);
			info("--receive-timeout   -rt  seconds to wait for receiving   (%hhu)\n", receive_timeout);
			info("--send-timeout      -st  seconds to wait for sending     (%hhu)\n", send_timeout);
			info("--log-level         -ll  logging verbosity to print      (%s)\n", human_log_level(log_level));
			info("--log-requests      -lq  log incoming requests           (%s)\n", human_bool(log_requests));
			info("--log-responses     -ls  log outgoing response           (%s)\n", human_bool(log_responses));
			exit(0);
		} else if (strcmp(flag, "--version") == 0 || strcmp(flag, "-v") == 0) {
			info("flua flights version 0.17.10\n");
			info("written by simylein in c\n");
			exit(0);
		} else if (strcmp(flag, "--address") == 0 || strcmp(flag, "-a") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_str(arg, "address", 4, 16, &address);
		} else if (strcmp(flag, "--port") == 0 || strcmp(flag, "-p") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint16(arg, "port", 0, 65535, &port);
		} else if (strcmp(flag, "--backlog") == 0 || strcmp(flag, "-b") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint8(arg, "backlog", 0, 255, &backlog);
		} else if (strcmp(flag, "--queue-size") == 0 || strcmp(flag, "-qs") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint8(arg, "queue size", 0, 127, &queue_size);
		} else if (strcmp(flag, "--least-workers") == 0 || strcmp(flag, "-lw") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint8(arg, "least-workers", 1, 63, &least_workers);
		} else if (strcmp(flag, "--most-workers") == 0 || strcmp(flag, "-mw") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint8(arg, "most-workers", 3, 255, &most_workers);
		} else if (strcmp(flag, "--bwt-ttl") == 0 || strcmp(flag, "-bt") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint32(arg, "bwt ttl", 3600, 15768000, &bwt_ttl);
		} else if (strcmp(flag, "--bwt-key") == 0 || strcmp(flag, "-bk") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_str(arg, "bwt key", 16, 64, &bwt_key);
		} else if (strcmp(flag, "--database-file") == 0 || strcmp(flag, "-df") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_str(arg, "database file", 4, 64, &database_file);
		} else if (strcmp(flag, "--database-timeout") == 0 || strcmp(flag, "-dt") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint16(arg, "database timeout", 0, 8000, &database_timeout);
		} else if (strcmp(flag, "--receive-timeout") == 0 || strcmp(flag, "-rt") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint8(arg, "receive timeout", 0, 240, &receive_timeout);
		} else if (strcmp(flag, "--send-timeout") == 0 || strcmp(flag, "-st") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_uint8(arg, "send timeout", 0, 240, &send_timeout);
		} else if (strcmp(flag, "--log-level") == 0 || strcmp(flag, "-ll") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_log_level(arg, "log level", &log_level);
		} else if (strcmp(flag, "--log-requests") == 0 || strcmp(flag, "-lq") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_bool(arg, "log requests", &log_requests);
		} else if (strcmp(flag, "--log-responses") == 0 || strcmp(flag, "-ls") == 0) {
			const char *arg = next_arg(argc, argv, &ind);
			errors += parse_bool(arg, "log responses", &log_responses);
		} else {
			errors++;
			error("unknown argument %s\n", flag);
		}
	}

	return errors;
}

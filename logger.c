#include "config.h"
#include "error.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

const char *purple = "\x1b[35m";
const char *blue = "\x1b[34m";
const char *cyan = "\x1b[36m";
const char *green = "\x1b[32m";
const char *yellow = "\x1b[33m";
const char *red = "\x1b[31m";
const char *bold = "\x1b[1m";
const char *normal = "\x1b[22m";
const char *reset = "\x1b[39m";

FILE *req_file = NULL;
FILE *res_file = NULL;
FILE *trace_file = NULL;
FILE *debug_file = NULL;
FILE *info_file = NULL;
FILE *warn_file = NULL;
FILE *error_file = NULL;
FILE *fatal_file = NULL;

void timestamp(char (*buffer)[9]) {
	time_t now = time(NULL);
	int elapsed = now % 86400;
	int seconds = elapsed % 60;
	int minutes = elapsed / 60 % 60;
	int hours = elapsed / 3600;
	(*buffer)[0] = (char)hours / 10 + 48;
	(*buffer)[1] = (char)hours % 10 + 48;
	(*buffer)[2] = ':';
	(*buffer)[3] = (char)minutes / 10 + 48;
	(*buffer)[4] = (char)minutes % 10 + 48;
	(*buffer)[5] = ':';
	(*buffer)[6] = (char)seconds / 10 + 48;
	(*buffer)[7] = (char)seconds % 10 + 48;
	(*buffer)[8] = '\0';
}

void print_color(FILE *file, const char *level, const char *color, const char *message, va_list args) {
	char buffer[9];
	timestamp(&buffer);
	flockfile(file);
	fprintf(file, "%s%sflua%s%s %s %s%s%s%s%s ", bold, blue, reset, normal, buffer, bold, color, level, reset, normal);
	vfprintf(file, message, args);
	funlockfile(file);
}

void req(const char *message, ...) {
	if (log_requests >= 1) {
		va_list args;
		va_start(args, message);
		print_color(stdout, "req", reset, message, args);
		va_end(args);
	}
}

void res(const char *message, ...) {
	if (log_responses >= 1) {
		va_list args;
		va_start(args, message);
		print_color(stdout, "res", reset, message, args);
		va_end(args);
	}
}

void trace(const char *message, ...) {
	if (log_level >= 6) {
		va_list args;
		va_start(args, message);
		print_color(stdout, "trace", blue, message, args);
		va_end(args);
	}
}

void debug(const char *message, ...) {
	if (log_level >= 5) {
		va_list args;
		va_start(args, message);
		print_color(stdout, "debug", cyan, message, args);
		va_end(args);
	}
}

void info(const char *message, ...) {
	if (log_level >= 4) {
		va_list args;
		va_start(args, message);
		print_color(stdout, "info", green, message, args);
		va_end(args);
	}
}

void warn(const char *message, ...) {
	if (log_level >= 3) {
		va_list args;
		va_start(args, message);
		print_color(stderr, "warn", yellow, message, args);
		va_end(args);
	}
}

void error(const char *message, ...) {
	if (log_level >= 2) {
		va_list args;
		va_start(args, message);
		print_color(stderr, "error", red, message, args);
		va_end(args);
	}
}

void fatal(const char *message, ...) {
	if (log_level >= 1) {
		va_list args;
		va_start(args, message);
		print_color(stderr, "fatal", purple, message, args);
		va_end(args);
	}
}

FILE *logfile(const char *log_path, int *errors) {
	int fd = open(log_path, O_RDONLY);
	if (fd != -1) {
		close(fd);
		trace("opening %s log file\n", log_path);
		FILE *file = fopen(log_path, "a");
		if (file == NULL) {
			error("%s\n", errno_str());
			error("failed to open %s\n", log_path);
			(*errors)++;
		}
		return file;
	}
}

int logfiles(const char *req_log, const char *res_log, const char *trace_log, const char *debug_log, const char *info_log,
						 const char *warn_log, const char *error_log, const char *fatal_log) {
	int errors = 0;
	req_file = logfile(req_log, &errors);
	res_file = logfile(res_log, &errors);
	trace_file = logfile(trace_log, &errors);
	debug_file = logfile(debug_log, &errors);
	info_file = logfile(info_log, &errors);
	warn_file = logfile(warn_log, &errors);
	error_file = logfile(error_log, &errors);
	fatal_file = logfile(fatal_log, &errors);
	return errors;
}

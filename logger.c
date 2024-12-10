#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

const char *purple = "\x1b[35m";
const char *blue = "\x1b[34m";
const char *cyan = "\x1b[36m";
const char *green = "\x1b[32m";
const char *yellow = "\x1b[33m";
const char *red = "\x1b[31m";
const char *bold = "\x1b[1m";
const char *normal = "\x1b[22m";
const char *reset = "\x1b[39m";

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

void print(FILE *file, const char *level, const char *color, const char *message, va_list args) {
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
		print(stdout, "req", reset, message, args);
		va_end(args);
	}
}

void res(const char *message, ...) {
	if (log_responses >= 1) {
		va_list args;
		va_start(args, message);
		print(stdout, "res", reset, message, args);
		va_end(args);
	}
}

void trace(const char *message, ...) {
	if (log_level >= 6) {
		va_list args;
		va_start(args, message);
		print(stdout, "trace", blue, message, args);
		va_end(args);
	}
}

void debug(const char *message, ...) {
	if (log_level >= 5) {
		va_list args;
		va_start(args, message);
		print(stdout, "debug", cyan, message, args);
		va_end(args);
	}
}

void info(const char *message, ...) {
	if (log_level >= 4) {
		va_list args;
		va_start(args, message);
		print(stdout, "info", green, message, args);
		va_end(args);
	}
}

void warn(const char *message, ...) {
	if (log_level >= 3) {
		va_list args;
		va_start(args, message);
		print(stderr, "warn", yellow, message, args);
		va_end(args);
	}
}

void error(const char *message, ...) {
	if (log_level >= 2) {
		va_list args;
		va_start(args, message);
		print(stderr, "error", red, message, args);
		va_end(args);
	}
}

void fatal(const char *message, ...) {
	if (log_level >= 1) {
		va_list args;
		va_start(args, message);
		print(stderr, "fatal", purple, message, args);
		va_end(args);
	}
}

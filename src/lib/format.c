#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

const char *human_bool(bool val) {
	if (val) {
		return "true";
	}
	return "false";
}

const char *human_log_level(uint8_t level) {
	switch (level) {
	case 1:
		return "fatal";
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

void human_bytes(char (*buffer)[8], size_t bytes) {
	if (bytes < 1000) {
		sprintf(*buffer, "%zub", bytes);
	} else if (bytes < 10000) {
		sprintf(*buffer, "%.1fkb", (float)bytes / 1000);
	} else if (bytes < 1000000) {
		sprintf(*buffer, "%zukb", bytes / 1000);
	} else if (bytes < 10000000) {
		sprintf(*buffer, "%.1fmb", (float)bytes / 1000000);
	} else if (bytes < 1000000000) {
		sprintf(*buffer, "%zumb", bytes / 1000000);
	} else if (bytes < 10000000000) {
		sprintf(*buffer, "%.1fgb", (float)bytes / 1000000000);
	} else {
		sprintf(*buffer, "%zugb", bytes / 1000000000);
	}
}

void human_duration(char (*buffer)[8], struct timespec *start, struct timespec *stop) {
	time_t start_nanoseconds = start->tv_sec * 1000000000 + start->tv_nsec;
	time_t stop_nanoseconds = stop->tv_sec * 1000000000 + stop->tv_nsec;
	time_t nanoseconds = stop_nanoseconds - start_nanoseconds;
	if (nanoseconds < 1000) {
		sprintf(*buffer, "%zuns", nanoseconds);
	} else if (nanoseconds < 10000) {
		sprintf(*buffer, "%.1fus", (float)nanoseconds / 1000);
	} else if (nanoseconds < 1000000) {
		sprintf(*buffer, "%zuus", nanoseconds / 1000);
	} else if (nanoseconds < 10000000) {
		sprintf(*buffer, "%.1fms", (float)nanoseconds / 1000000);
	} else if (nanoseconds < 1000000000) {
		sprintf(*buffer, "%zums", nanoseconds / 1000000);
	} else if (nanoseconds < 10000000000) {
		sprintf(*buffer, "%.1fs", (float)nanoseconds / 1000000000);
	} else {
		sprintf(*buffer, "%zus", nanoseconds / 1000000000);
	}
}

void human_time(char (*buffer)[8], time_t seconds) {
	if (seconds < 60) {
		sprintf(*buffer, "%zus", seconds);
	} else if (seconds < 3600) {
		sprintf(*buffer, "%zum", seconds / 60);
	} else if (seconds < 86400) {
		sprintf(*buffer, "%zuh", seconds / 3600);
	} else {
		sprintf(*buffer, "%zud", seconds / 86400);
	}
}

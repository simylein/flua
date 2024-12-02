#include <stdint.h>
#include <stdio.h>
#include <time.h>

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

void human_duration(char (*buffer)[8], struct timespec *start, struct timespec *stop) {
	time_t start_nanoseconds = start->tv_sec * 1000000000 + start->tv_nsec;
	time_t stop_nanoseconds = stop->tv_sec * 1000000000 + stop->tv_nsec;
	time_t nanoseconds = stop_nanoseconds - start_nanoseconds;
	if (nanoseconds < 1000) {
		sprintf(*buffer, "%zuns", nanoseconds);
	} else if (nanoseconds < 1000000) {
		sprintf(*buffer, "%zuus", nanoseconds / 1000);
	} else if (nanoseconds < 1000000000) {
		sprintf(*buffer, "%zums", nanoseconds / 1000000);
	} else {
		sprintf(*buffer, "%zus", nanoseconds / 1000000000);
	}
}

void human_bytes(char (*buffer)[8], size_t bytes) {
	if (bytes < 1000) {
		sprintf(*buffer, "%zub", bytes);
	} else if (bytes < 1000000) {
		sprintf(*buffer, "%zukb", bytes / 1000);
	} else if (bytes < 1000000000) {
		sprintf(*buffer, "%zumb", bytes / 1000000);
	} else {
		sprintf(*buffer, "%zugb", bytes / 1000000000);
	}
}

int bin_to_hex(char *buffer, const size_t buffer_len, const void *bin, const size_t bin_len) {
	if (bin_len != (buffer_len - 1) / 2) {
		return -1;
	}
	size_t index = 0;
	while (index < (buffer_len - 1) / 2) {
		sprintf(buffer + index * 2, "%02x", ((const uint8_t *)bin)[index]);
		index++;
	}
	buffer[index * 2] = '\0';
	return 0;
}

int hex_to_bin(void *buffer, const size_t buffer_len, const char *hex, const size_t hex_len) {
	if (hex_len != buffer_len * 2) {
		return -1;
	}
	size_t index = 0;
	while (index < buffer_len) {
		unsigned int byte;
		if (sscanf(hex + index * 2, "%2x", &byte) != 1) {
			return -1;
		}
		((uint8_t *)buffer)[index] = (uint8_t)byte;
		index++;
	}
	return 0;
}

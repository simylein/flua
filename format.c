#include <stdio.h>
#include <time.h>

void human_duration(char (*buffer)[8], struct timespec *start, struct timespec *stop) {
	time_t start_nanoseconds = start->tv_sec * 1000000000 + start->tv_nsec;
	time_t stop_nanoseconds = stop->tv_sec * 1000000000 + stop->tv_nsec;
	time_t nanoseconds = stop_nanoseconds - start_nanoseconds;
	if (nanoseconds < 1000) {
		sprintf(*buffer, "%ldns", nanoseconds);
	} else if (nanoseconds < 1000000) {
		sprintf(*buffer, "%ldus", nanoseconds / 1000);
	} else if (nanoseconds < 1000000000) {
		sprintf(*buffer, "%ldms", nanoseconds / 1000000);
	} else {
		sprintf(*buffer, "%lds", nanoseconds / 1000000000);
	}
}

void human_bytes(char (*buffer)[8], size_t bytes) {
	if (bytes < 1000) {
		sprintf(*buffer, "%ldb", bytes);
	} else if (bytes < 1000000) {
		sprintf(*buffer, "%ldkb", bytes / 1000);
	} else if (bytes < 1000000000) {
		sprintf(*buffer, "%ldmb", bytes / 1000000);
	} else {
		sprintf(*buffer, "%ldgb", bytes / 1000000000);
	}
}

int bin_to_hex(char *buffer, const size_t buffer_size, const unsigned char *bin, const int bin_size) {
	if (bin_size != (buffer_size - 1) / 2) {
		return -1;
	}
	size_t index = 0;
	while (index < (buffer_size - 1) / 2) {
		sprintf(buffer + index * 2, "%02x", bin[index]);
		index++;
	}
	buffer[index * 2] = '\0';
	return 0;
}

int hex_to_bin(unsigned char *buffer, const size_t buffer_size, const char *hex, const size_t hex_size) {
	if (hex_size != buffer_size * 2) {
		return -1;
	}
	size_t index = 0;
	while (index < buffer_size) {
		unsigned int byte;
		if (sscanf(hex + index * 2, "%2x", &byte) != 1) {
			return -1;
		}
		buffer[index] = (unsigned char)byte;
		index++;
	}
	return 0;
}

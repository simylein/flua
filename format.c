#include <stdio.h>
#include <time.h>

char *human_duration(struct timespec *start, struct timespec *stop) {
	time_t start_nanoseconds = start->tv_sec * 1000000000 + start->tv_nsec;
	time_t stop_nanoseconds = stop->tv_sec * 1000000000 + stop->tv_nsec;
	time_t nanoseconds = stop_nanoseconds - start_nanoseconds;
	static char buffer[8];
	if (nanoseconds < 1000) {
		sprintf(buffer, "%ldns", nanoseconds);
	} else if (nanoseconds < 1000000) {
		sprintf(buffer, "%ldus", nanoseconds / 1000);
	} else if (nanoseconds < 1000000000) {
		sprintf(buffer, "%ldms", nanoseconds / 1000000);
	} else {
		sprintf(buffer, "%lds", nanoseconds / 1000000000);
	}
	return buffer;
}

char *human_bytes(size_t bytes) {
	static char buffer[8];
	if (bytes < 1000) {
		sprintf(buffer, "%ldb", bytes);
	} else if (bytes < 1000000) {
		sprintf(buffer, "%ldkb", bytes / 1000);
	} else if (bytes < 1000000000) {
		sprintf(buffer, "%ldmb", bytes / 1000000);
	} else {
		sprintf(buffer, "%ldgb", bytes / 1000000000);
	}
	return buffer;
}

int human_uuid(char (*buffer)[33], const unsigned char *binary_uuid, const int binary_uuid_size) {
	if (binary_uuid_size != (sizeof(*buffer) - 1) / 2) {
		return -1;
	}
	size_t index = 0;
	while (index < (sizeof(*buffer) - 1) / 2) {
		sprintf(*buffer + index * 2, "%02x", binary_uuid[index]);
		index++;
	}
	(*buffer)[index * 2] = '\0';
	return 0;
}

int binary_uuid(unsigned char (*buffer)[16], const char *hex_uuid, const size_t hex_uuid_size) {
	if (hex_uuid_size != sizeof(*buffer) * 2) {
		return -1;
	}
	size_t index = 0;
	while (index < sizeof(*buffer)) {
		unsigned int byte;
		if (sscanf(hex_uuid + index * 2, "%2x", &byte) != 1) {
			return -1;
		}
		(*buffer)[index] = (unsigned char)byte;
		index++;
	}
	return 0;
}

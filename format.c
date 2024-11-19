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

const char *human_uuid(const unsigned char *binary_uuid, const int binary_uuid_size) {
	static char buffer[33];
	if (binary_uuid_size != (sizeof(buffer) - 1) / 2) {
		return NULL;
	}
	size_t index = 0;
	while (index < (sizeof(buffer) - 1) / 2) {
		sprintf(&buffer[index * 2], "%02x", binary_uuid[index]);
		index++;
	}
	buffer[index * 2] = '\0';
	return buffer;
}

const unsigned char *binary_uuid(const char *uuid, const size_t uuid_size) {
	static unsigned char buffer[16];
	if (uuid_size != sizeof(buffer) * 2) {
		return NULL;
	}
	size_t index = 0;
	while (index < sizeof(buffer)) {
		unsigned int byte;
		if (sscanf(&uuid[index * 2], "%2x", &byte) != 1) {
			return NULL;
		}
		buffer[index] = (unsigned char)byte;
		index++;
	}
	return buffer;
}

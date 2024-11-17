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

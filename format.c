#include <stdio.h>
#include <time.h>

char *human_duration(struct timespec *start, struct timespec *stop) {
	time_t start_nanoseconds = start->tv_sec * 1000000000 + start->tv_nsec;
	time_t stop_nanoseconds = stop->tv_sec * 1000000000 + stop->tv_nsec;
	time_t nanoseconds = stop_nanoseconds - start_nanoseconds;
	static char duration_buffer[8];
	if (nanoseconds < 1000) {
		snprintf(duration_buffer, sizeof(duration_buffer), "%ldns", nanoseconds);
	} else if (nanoseconds < 1000000) {
		snprintf(duration_buffer, sizeof(duration_buffer), "%ldus", nanoseconds / 1000);
	} else if (nanoseconds < 1000000000) {
		snprintf(duration_buffer, sizeof(duration_buffer), "%ldms", nanoseconds / 1000000);
	} else {
		snprintf(duration_buffer, sizeof(duration_buffer), "%lds", nanoseconds / 1000000000);
	}
	return duration_buffer;
}

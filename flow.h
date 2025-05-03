#include "request.h"
#include "response.h"
#include <sqlite3.h>
#include <stdint.h>
#include <time.h>

#pragma once
typedef struct flow_t {
	time_t timestamp;
	char method[8];
	uint8_t method_len;
	char pathname[128];
	uint8_t pathname_len;
	char search[256];
	uint16_t search_len;
	uint16_t status;
	time_t duration;
	ssize_t bytes_received;
	ssize_t bytes_sent;
} flow_t;

void *flowing(void *args);

void stash_flow(time_t timestamp, request_t *request, response_t *response, struct timespec *start, struct timespec *stop,
								ssize_t bytes_received, ssize_t bytes_sent);

void create_flow(sqlite3 *database, flow_t *flow);

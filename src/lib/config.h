#pragma once

#include <stdbool.h>
#include <stdint.h>

extern const char *name;

extern const char *address;
extern uint16_t port;

extern uint8_t backlog;
extern uint8_t queue_size;
extern uint8_t least_workers;
extern uint8_t most_workers;

extern char *bwt_key;
extern uint32_t bwt_ttl;

extern const char *database_file;
extern uint16_t database_timeout;

extern uint8_t receive_timeout;
extern uint8_t send_timeout;
extern uint8_t receive_packets;
extern uint8_t send_packets;
extern uint32_t receive_buffer;
extern uint32_t send_buffer;

extern uint8_t log_level;
extern bool log_requests;
extern bool log_responses;

int configure(int argc, char *argv[], uint8_t *cmds);

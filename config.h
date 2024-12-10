#include <stdbool.h>
#include <stdint.h>

extern char *address;
extern uint16_t port;

extern uint8_t backlog;
extern uint8_t workers;
extern uint8_t queue_size;

extern uint32_t bwt_ttl;
extern char *bwt_key;

extern char *database_file;
extern uint16_t database_timeout;

extern uint8_t log_level;
extern bool log_requests;
extern bool log_responses;

int configure(int argc, char *argv[]);

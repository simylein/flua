#include <stdint.h>

extern char *address;
extern uint16_t port;

extern uint8_t backlog;
extern uint8_t workers;
extern uint8_t queue_size;

extern int bwt_ttl;
extern char *bwt_key;

extern char *database_file;
extern uint16_t database_timeout;

extern int log_level;
extern int log_requests;
extern int log_responses;

int configure(int argc, char *argv[]);

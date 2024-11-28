extern char *address;
extern int port;

extern int backlog;
extern int workers;

extern int jwt_ttl;

extern char *database_file;

extern int log_level;
extern int log_requests;
extern int log_responses;

int configure(int argc, char *argv[]);

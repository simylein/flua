int port;

int backlog;
int workers;

int jwt_ttl;
int cache_ttl;

char *database_file;

int log_level;
int log_requests;
int log_responses;

int configure(int argc, char *argv[]);

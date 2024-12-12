#include <arpa/inet.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdint.h>

typedef struct arg_t {
	size_t id;
	sqlite3 *database;
} arg_t;

typedef struct task_t {
	int client_sock;
	struct sockaddr_in client_addr;
} task_t;

typedef struct queue_t {
	task_t *tasks;
	size_t front;
	size_t back;
	size_t size;
	size_t load;
	pthread_mutex_t lock;
	pthread_cond_t filled;
	pthread_cond_t available;
} queue_t;

extern struct queue_t queue;

void *thread(void *args);

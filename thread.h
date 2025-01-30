#include <arpa/inet.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdint.h>

typedef struct arg_t {
	uint8_t id;
	sqlite3 *database;
} arg_t;

typedef struct task_t {
	int client_sock;
	struct sockaddr_in client_addr;
} task_t;

typedef struct queue_t {
	task_t *tasks;
	uint8_t head;
	uint8_t tail;
	uint8_t size;
	pthread_mutex_t lock;
	pthread_cond_t filled;
	pthread_cond_t available;
} queue_t;

extern struct queue_t queue;

typedef struct worker_t {
	arg_t arg;
	pthread_t thread;
} worker_t;

typedef struct thread_pool_t {
	worker_t *workers;
	uint8_t size;
	uint8_t load;
	pthread_mutex_t lock;
	pthread_cond_t available;
} thread_pool_t;

extern struct thread_pool_t thread_pool;

int spawn(worker_t *workers, uint8_t index, void (*logger)(const char *message, ...) __attribute__((format(printf, 1, 2))));

void *thread(void *args);

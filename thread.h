#include <arpa/inet.h>
#include <pthread/pthread.h>

typedef struct Task {
	int client_sock;
	struct sockaddr_in client_addr;
} Task;

typedef struct Queue {
	Task *tasks;
	size_t front;
	size_t back;
	size_t size;
	pthread_mutex_t lock;
	pthread_cond_t filled;
	pthread_cond_t available;
} Queue;

extern struct Queue queue;

void *thread(void *arg);

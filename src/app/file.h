#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef struct file_t {
	int fd;
	char *ptr;
	size_t len;
	const char *path;
	time_t modified;
	bool hydrated;
	pthread_rwlock_t lock;
} file_t;

const char *type(const char *path);

int file(file_t *file, uint8_t (*cull)(file_t *file));

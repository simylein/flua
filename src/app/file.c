#include "file.h"
#include "../lib/error.h"
#include "../lib/logger.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

time_t mtime(struct stat *file_stat) {
#ifdef __APPLE__
	return file_stat->st_mtimespec.tv_sec;
#elif __linux__
	return file_stat->st_mtim.tv_sec;
#else
#error "unsupported platform"
#endif
}

const char *type(const char *path) {
	const char *extension = strrchr(path, '.');
	if (extension == NULL) {
		error("file path %s has no extension\n", path);
		return "text/unknown";
	}
	if (strcmp(extension, ".txt") == 0) {
		return "text/plain";
	}
	if (strcmp(extension, ".html") == 0) {
		return "text/html";
	}
	error("unknown content type %s\n", extension);
	return "unknown";
}

int file(file_t *file, uint8_t (*cull)(file_t *file)) {
	int status = 0;

	pthread_rwlock_rdlock(&file->lock);
	if (file->fd == -1) {
		pthread_rwlock_unlock(&file->lock);
		pthread_rwlock_wrlock(&file->lock);
		if (file->fd != -1) {
			status = 0;
			goto cleanup;
		}

		trace("opening file %s\n", file->path);

		file->fd = open(file->path, O_RDONLY);
		if (file->fd == -1) {
			error("failed to open %s because %s\n", file->path, errno_str());
			status = -1;
			goto cleanup;
		}
	}

	struct stat file_stat;
	if (fstat(file->fd, &file_stat) == -1) {
		error("failed to stat %s because %s\n", file->path, errno_str());
		status = -1;
		goto cleanup;
	}

	time_t modified = mtime(&file_stat);
	if (file->ptr == NULL || file->modified != modified) {
		pthread_rwlock_unlock(&file->lock);
		pthread_rwlock_wrlock(&file->lock);
		if (file->ptr != NULL && file->modified == modified) {
			status = 0;
			goto cleanup;
		}

		debug("reading file %s\n", file->path);

		file->ptr = realloc(file->ptr, (size_t)file_stat.st_size);
		if (file->ptr == NULL) {
			error("failed to allocate %zu bytes for %s because %s\n", (size_t)file_stat.st_size, file->path, errno_str());
			status = -1;
			goto cleanup;
		}

		if (lseek(file->fd, 0, SEEK_SET) == -1) {
			error("failed to seek to start of %s because %s\n", file->path, errno_str());
			status = -1;
			goto cleanup;
		}

		ssize_t bytes_read = read(file->fd, file->ptr, (size_t)file_stat.st_size);
		if (bytes_read != file_stat.st_size) {
			error("failed to read %zu bytes from %s because %s\n", (size_t)(file_stat.st_size - bytes_read), file->path, errno_str());
			status = -1;
			goto cleanup;
		}

		file->len = (size_t)file_stat.st_size;
		file->modified = modified;
		file->hydrated = false;
		if (cull != NULL) {
			uint8_t frags = cull(file);
			if (frags != 0) {
				warn("cull failed to remove %hhu fragments\n", frags);
			}
		}
	}

	status = 0;

cleanup:
	pthread_rwlock_unlock(&file->lock);
	return status;
}

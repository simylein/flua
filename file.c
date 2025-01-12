#include "file.h"
#include "config.h"
#include "error.h"
#include "logger.h"
#include "response.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

char *type(const char *file_path) {
	const char *extension = strstr(file_path, ".");
	if (extension == NULL) {
		error("file path %s has no extension\n", file_path);
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

void file(const char *file_path, file_t *file, response_t *response) {
	if (file->fd == -1) {
		trace("opening file %s\n", file_path);

		file->fd = open(file_path, O_RDONLY);
		if (file->fd == -1) {
			error("failed to open %s because %s\n", file_path, errno_str());
			response->status = 500;
			return;
		}
	}

	struct stat file_stat;
	if (fstat(file->fd, &file_stat) == -1) {
		error("failed to stat %s because %s\n", file_path, errno_str());
		response->status = 500;
		return;
	}

	if (file->ptr == NULL || file->age != (time_t)file_stat.st_mtimespec.tv_sec) {
		debug("reading file %s\n", file_path);

		if ((size_t)file_stat.st_size > sizeof(response->body)) {
			error("file %s size %zu is too large\n", file_path, (size_t)file_stat.st_size);
			response->status = 500;
			return;
		}

		file->ptr = realloc(file->ptr, (size_t)file_stat.st_size);
		if (file->ptr == NULL) {
			error("failed to allocate %zu bytes for %s because %s\n", (size_t)file_stat.st_size, file_path, errno_str());
			response->status = 500;
			return;
		}

		if (lseek(file->fd, 0, SEEK_SET) == -1) {
			error("failed to seek to start of %s because %s\n", file_path, errno_str());
			response->status = 500;
			return;
		}

		ssize_t bytes_read = read(file->fd, file->ptr, (size_t)file_stat.st_size);
		if (bytes_read != file_stat.st_size) {
			error("failed to read %zu bytes from %s because %s\n", (size_t)(file_stat.st_size - bytes_read), file_path, errno_str());
			response->status = 500;
			return;
		}

		file->len = (size_t)file_stat.st_size;
		file->age = (time_t)file_stat.st_mtimespec.tv_sec;
	}

	if (file->ptr != NULL) {
		info("sending file %s\n", file_path);

		if (response->status == 0) {
			response->status = 200;
		}
		append_header(response, "content-type:%s\r\n", type(file_path));
		append_header(response, "content-length:%zu\r\n", file->len);
		append_body(response, file->ptr, file->len);
	}
}

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
	return "text/unknown";
}

void file(const char *file_path, file_t *file, response_t *response) {
	if (file->ptr == NULL) {
		debug("reading file %s\n", file_path);

		int file_fd = open(file_path, O_RDONLY);
		if (file_fd == -1) {
			error("%s\n", errno_str());
			error("failed to open %s\n", file_path);
			response->status = 500;
			return;
		}

		struct stat file_stat;
		if (fstat(file_fd, &file_stat) == -1) {
			error("%s\n", errno_str());
			error("failed to stat %s\n", file_path);
			response->status = 500;
			goto cleanup;
		}

		if ((size_t)file_stat.st_size > sizeof(response->body)) {
			error("file %s size %zu is too large\n", file_path, (size_t)file_stat.st_size);
			response->status = 500;
			goto cleanup;
		}

		file->ptr = malloc((size_t)file_stat.st_size);
		if (file->ptr == NULL) {
			error("%s\n", errno_str());
			error("failed to allocate %zu bytes for %s\n", (size_t)file_stat.st_size, file_path);
			response->status = 500;
			goto cleanup;
		}

		ssize_t bytes_read = read(file_fd, file->ptr, (size_t)file_stat.st_size);
		if (bytes_read != file_stat.st_size) {
			error("%s\n", errno_str());
			error("failed to fully read %s\n", file_path);
			response->status = 500;
			goto cleanup;
		}

		file->len = (size_t)file_stat.st_size;

	cleanup:
		close(file_fd);
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

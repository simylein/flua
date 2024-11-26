#include "config.h"
#include "error.h"
#include "logger.h"
#include "response.h"
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char *type(const char *file_path) {
	const char *extension = strstr(file_path, ".");
	if (extension == NULL) {
		error("file path %s has no extension\n", file_path);
		return "text/unknown";
	}
	if (strcmp(extension, ".html") == 0) {
		return "text/html";
	}
	if (strcmp(extension, ".css") == 0) {
		return "text/css";
	}
	if (strcmp(extension, ".js") == 0) {
		return "text/javascript";
	}
	error("unknown content type %s\n", extension);
	return "text/unknown";
}

void file(const char *file_path, Response *response) {
	info("sending file %s\n", file_path);

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
		error("file %s exceeds buffer\n", file_path);
		response->status = 500;
		goto cleanup;
	}

	ssize_t bytes_read = read(file_fd, response->body, (size_t)file_stat.st_size);
	if (bytes_read != file_stat.st_size) {
		error("%s\n", errno_str());
		error("failed to read %s\n", file_path);
		response->status = 500;
		goto cleanup;
	}

	if (response->status == 0) {
		response->status = 200;
	}
	response->header_len = (size_t)sprintf(response->header, "content-type:%s\r\n", type(file_path));
	response->header_len = (size_t)sprintf(response->header, "content-length:%zu\r\n", bytes_read);
	response->header_len = (size_t)sprintf(response->header, "cache-control:max-age=%d\r\n\r\n", cache_ttl);
	response->body_len += (size_t)bytes_read;

cleanup:
	close(file_fd);
}

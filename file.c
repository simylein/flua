#include "error.h"
#include "logger.h"
#include "response.h"
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void file(const char *file_path, Response *response) {
	char buffer[6144] = {0};

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
		close(file_fd);
		response->status = 500;
		return;
	}

	if ((size_t)file_stat.st_size > sizeof(buffer)) {
		error("file %s exceeds buffer\n", file_path);
		close(file_fd);
		response->status = 500;
		return;
	}

	ssize_t bytes_read = read(file_fd, buffer, (size_t)file_stat.st_size);
	if (bytes_read != file_stat.st_size) {
		error("%s\n", errno_str());
		error("failed to read %s\n", file_path);
		close(file_fd);
		response->status = 500;
		return;
	}

	trace("sending file %s\n", file_path);
	sprintf(&*response->header, "content-type:text/html\r\ncontent-length:%ld\r\n\r\n", bytes_read);
	memcpy(response->body, buffer, bytes_read);
	close(file_fd);
}

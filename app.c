#include "error.h"
#include "logger.h"
#include "request.h"
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

void handle(Request *request, Response *response) {
	int method_found = 0;
	int pathname_found = 0;

	if (strcmp(request->pathname, "/") == 0) {
		pathname_found = 1;
		if (strcmp(request->method, "get") == 0) {
			method_found = 1;
			if (strlen(request->search) > 0) {
				response->status = 400;
			} else {
				response->status = 200;
				file("home.html", response);
			}
		}
	}

	if (pathname_found == 0 && method_found == 0) {
		response->status = 404;
	}
	if (pathname_found == 1 && method_found == 0) {
		response->status = 405;
	}

	if (response->status == 400) {
		file("400.html", response);
	}
	if (response->status == 404) {
		file("404.html", response);
	}
	if (response->status == 405) {
		file("405.html", response);
	}
	if (response->status == 500) {
		file("500.html", response);
	}
}

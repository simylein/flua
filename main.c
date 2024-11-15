#include "app.h"
#include "config.h"
#include "error.h"
#include "format.h"
#include "logger.h"
#include <arpa/inet.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int cf_errors = configure(argc, argv);
  if (cf_errors > 0) {
    fatal("config contains %d errors\n", cf_errors);
    return EXIT_FAILURE;
  }
  if (cf_errors == -1) {
    return EXIT_SUCCESS;
  }

  sqlite3 *database;

  int db_error = sqlite3_open_v2(database_file, &database, SQLITE_OPEN_READWRITE, NULL);
  if (db_error != 0) {
    error("%s\n", sqlite3_errmsg(database));
    fatal("failed to open database %s\n", database_file);
    return EXIT_FAILURE;
  }

  info("using database %s\n", database_file);

  int server_sock;
  struct sockaddr_in server_addr;

  if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    error("%s\n", errno_str());
    fatal("failed to create socket\n");
    return EXIT_FAILURE;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    error("%s\n", errno_str());
    fatal("failed to bind to socket\n");
    return EXIT_FAILURE;
  }

  if (listen(server_sock, backlog) == -1) {
    error("%s\n", errno_str());
    fatal("failed to listen on socket\n");
    return EXIT_FAILURE;
  }

  info("listening on %s:%d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

  while (1) {
    struct sockaddr_in client_addr;
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &(socklen_t){sizeof(client_addr)});

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (client_sock == -1) {
      error("%s\n", errno_str());
      error("failed to accept client\n");
      continue;
    }

    char request_buffer[2048] = {0};
    ssize_t bytes_received = recv(client_sock, request_buffer, sizeof(request_buffer), 0);
    trace("received %zd bytes from %s:%d\n", bytes_received, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    if (bytes_received == -1) {
      error("%s\n", errno_str());
      error("failed to receive data from client\n");
      close(client_sock);
      continue;
    }

    struct Request reqs = request(&request_buffer, bytes_received);
    req("%s %s\n", reqs.method, reqs.pathname);

    struct Response resp = {.status = 0};
    if (reqs.status == 0) {
      handle(&reqs, &resp);
    } else {
      resp.status = reqs.status;
    }

    char response_buffer[2048] = {0};
    int response_length = response(&response_buffer, &resp);

    ssize_t bytes_sent = send(client_sock, response_buffer, (size_t)response_length, 0);
    trace("sent %zd bytes to %s:%d\n", bytes_sent, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    struct timespec stop;
    clock_gettime(CLOCK_MONOTONIC, &stop);

    res("%d %s\n", resp.status, human_duration(&start, &stop));

    if (bytes_sent == -1) {
      error("%s\n", errno_str());
      error("failed to send data to client\n");
    }

    if (close(client_sock) == -1) {
      error("%s\n", errno_str());
      error("failed to close client socket\n");
    }
  }
}

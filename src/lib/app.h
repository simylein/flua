#pragma once

#include <arpa/inet.h>
#include <sqlite3.h>

void handle(sqlite3 *database, char *request_buffer, char *response_buffer, int *client_sock, struct sockaddr_in *client_addr);

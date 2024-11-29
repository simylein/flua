#include <arpa/inet.h>
#include <sqlite3.h>

void handle(sqlite3 *database, int *client_sock, struct sockaddr_in *client_addr);

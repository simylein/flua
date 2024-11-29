#include "request.h"
#include "response.h"
#include <sqlite3.h>

int authenticate(request_t *request, char (*buffer)[33]);

void create_signin(sqlite3 *database, char *username, char *password, response_t *response);
void create_signup(sqlite3 *database, char *username, char *password, response_t *response);

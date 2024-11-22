#include "request.h"
#include "response.h"

int authenticate(Request *request, char (*buffer)[33]);

void create_signin(char *username, char *password, Response *response);
void create_signup(char *username, char *password, Response *response);

#include "request.h"
#include "response.h"

int authenticate(Request *request, char (*buffer)[33]);

void user_signin(char *username, char *password, Response *response);
void user_signup(char *username, char *password, Response *response);

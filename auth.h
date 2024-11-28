#include "request.h"
#include "response.h"

int authenticate(request_t *request, char (*buffer)[33]);

void create_signin(char *username, char *password, response_t *response);
void create_signup(char *username, char *password, response_t *response);

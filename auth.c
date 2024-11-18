#include "logger.h"
#include "response.h"

void user_signin(char *username, char *password, Response *response) {
	info("user %s signing in\n", username);

	debug("username: %s password: %s \n", username, password);
}
void user_signup(char *username, char *password, Response *response) {
	info("user %s signing up\n", username);

	debug("username: %s password: %s \n", username, password);
}

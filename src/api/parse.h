#include "../lib/request.h"
#include "flight.h"
#include "user.h"
#include <stdint.h>

int parse_credentials(char **username, uint8_t *username_len, char **password, uint8_t *password_len, request_t *request);
int validate_credentials(char **username, uint8_t *username_len, char **password, uint8_t *password_len);

int parse_user(user_t *user, request_t *request);
int validate_user(user_t *user);

int parse_flight(flight_t *flight, request_t *request);
int validate_flight(flight_t *flight);

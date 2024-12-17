#include "request.h"
#include <stdint.h>

int parse_credentials(char **username, uint8_t *username_len, char **password, uint8_t *password_len, request_t *request);
int validate_credentials(char **username, uint8_t *username_len, char **password, uint8_t *password_len);

int parse_flight(flight_t *flight, request_t *request);
int validate_flight(flight_t *flight);

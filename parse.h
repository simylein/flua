#include "request.h"

int parse_credentials(char (*username)[17], char (*password)[65], request_t *request);
int validate_credentials(char (*username)[17], char (*password)[65]);

int parse_flight(flight_t *flight, request_t *request);
int validate_flight(flight_t *flight);

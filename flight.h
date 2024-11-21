#include "response.h"

void find_flights(char *user_uuid, char *year, Response *response);
void find_flight_years(char *user_uuid, Response *response);
void create_flight(char *user_uuid, char *hash, uint64_t starts_at, uint64_t ends_at, Response *response);

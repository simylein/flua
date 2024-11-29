#include "response.h"
#include <sqlite3.h>

void find_flights(sqlite3 *database, char *user_uuid, char *year, response_t *response);
void find_flight_years(sqlite3 *database, char *user_uuid, response_t *response);
void create_flight(sqlite3 *database, char *user_uuid, char *hash, uint64_t starts_at, uint64_t ends_at, response_t *response);

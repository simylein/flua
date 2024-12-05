#include "bwt.h"
#include "response.h"
#include <sqlite3.h>
#include <stdint.h>

void find_years(sqlite3 *database, uint8_t (*user_id)[16], response_t *response);
void find_flights(sqlite3 *database, uint8_t (*user_id)[16], char *year, response_t *response);
void create_flight(sqlite3 *database, bwt_t *bwt, char *hash, uint64_t starts_at, uint64_t ends_at, response_t *response);

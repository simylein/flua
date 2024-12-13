#include "bwt.h"
#include "response.h"
#include <sqlite3.h>
#include <stdint.h>

typedef struct flight_t {
	uint8_t hash[32];
	uint64_t starts_at;
	uint64_t ends_at;
	uint16_t altitude[5];
	uint16_t thermal[5];
} flight_t;

void find_years(sqlite3 *database, uint8_t (*user_id)[16], response_t *response);
void find_flights(sqlite3 *database, uint8_t (*user_id)[16], char *year, size_t year_len, response_t *response);
void create_flight(sqlite3 *database, bwt_t *bwt, flight_t *flight, response_t *response);

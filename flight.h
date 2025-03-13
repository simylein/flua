#include "bwt.h"
#include "response.h"
#include <sqlite3.h>
#include <stdint.h>

#pragma once
typedef struct flight_t {
	uint8_t (*hash)[32];
	uint64_t *starts_at;
	uint64_t *ends_at;
	uint16_t (*altitude_bins)[5];
	uint16_t *altitude_min;
	uint16_t *altitude_max;
	uint16_t (*thermal_bins)[5];
	uint8_t *max_climb;
	uint8_t *max_sink;
	uint16_t (*speed_bins)[5];
	uint16_t *speed_avg;
	uint16_t *speed_max;
	uint16_t (*glide_bins)[5];
	uint32_t *distance_flown;
} flight_t;

void find_years(sqlite3 *database, uint8_t (*user_id)[16], response_t *response);
void find_flights(sqlite3 *database, uint8_t (*user_id)[16], char *year, size_t year_len, response_t *response);
void create_flight(sqlite3 *database, bwt_t *bwt, flight_t *flight, response_t *response);
void delete_flights(sqlite3 *database, bwt_t *bwt, response_t *response);

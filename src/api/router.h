#pragma once

#include "../lib/request.h"
#include "../lib/response.h"
#include <sqlite3.h>

void route(sqlite3 *database, request_t *request, response_t *response);

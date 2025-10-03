#pragma once

#include "../lib/bwt.h"
#include "../lib/request.h"
#include "../lib/response.h"
#include "file.h"
#include <sqlite3.h>

void serve(file_t *asset, uint8_t (*cull)(file_t *file), response_t *response);

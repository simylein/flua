#include "request.h"
#include "response.h"
#include <string.h>

void null_init(request_t *request, response_t *response);

char *strcasestr(const char *buffer, const char *buf);
char *strncasestrn(const char *buffer, size_t buffer_len, const char *buf, size_t buf_len);

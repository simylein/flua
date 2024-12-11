#include "request.h"
#include "response.h"
#include <string.h>

void null_init(request_t *request, response_t *response);

int strnfind(const char *buffer, const size_t buffer_len, const char *prefix, const char *suffix, char **string,
						 size_t *string_len, size_t string_max_len);

char *strcasestr(const char *buffer, const char *buf);
char *strncasestrn(const char *buffer, size_t buffer_len, const char *buf, size_t buf_len);

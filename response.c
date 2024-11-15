#include "response.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

int response(char (*buffer)[8192], Response *res) {
  int bytes = 0;
  bytes += snprintf(*buffer + bytes, (int)sizeof(*buffer) - bytes, "HTTP/1.1 %d %s\r\n", res->status, status_text(res->status));
  if (strlen(res->header) > 0) {
    bytes += snprintf(*buffer + bytes, (int)sizeof(*buffer) - bytes, "%s", res->header);
  }
  if (strlen(res->body) > 0) {
    bytes += snprintf(*buffer + bytes, (int)sizeof(*buffer) - bytes, "%s", res->body);
  }
  bytes += snprintf(*buffer + bytes, (int)sizeof(*buffer) - bytes, "\r\n");
  return bytes;
}

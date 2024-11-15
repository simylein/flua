#include "response.h"
#include <stdio.h>
#include <string.h>

char *status_text(int status) {
  switch (status) {
  case 200:
    return "OK";
  case 413:
    return "Content Too Large";
  case 414:
    return "URI Too Long";
  case 431:
    return "Request Header Fields Too Large";
  case 501:
    return "Not Implemented";
  case 505:
    return "HTTP Version Not Supported";
  default:
    return NULL;
  }
}

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

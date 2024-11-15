#include "response.h"
#include <stdio.h>

char *status_text(int status) {
  switch (status) {
  case 200:
    return "OK";
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

int response(char (*buffer)[2048], Response *res) {
  int bytes = snprintf(*buffer, sizeof(*buffer), "HTTP/1.1 %d %s\r\n\r\n", res->status, status_text(res->status));
  return bytes;
}

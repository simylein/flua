#include "request.h"

Request request(char (*buffer)[2048]) {
  struct Request req;

  unsigned short stage = 0;

  unsigned long global_index = 0;
  unsigned long method_index = 0;
  unsigned long pathname_index = 0;
  unsigned long search_index = 0;

  while (stage == 0 && method_index < sizeof(req.method) - 1 && global_index < sizeof(*buffer)) {
    char byte = (*buffer)[global_index];
    if (byte >= 'A' && byte <= 'Z') {
      req.method[method_index] = byte + 32;
    }
    if (byte == ' ') {
      stage = 1;
    }
    method_index++;
    global_index++;
  }
  req.method[method_index] = '\0';
  if (stage == 0) {
    req.status = 501;
    return req;
  }

  while (stage == 1 && pathname_index < sizeof(req.pathname) - 1 && global_index < sizeof(*buffer)) {
    char byte = (*buffer)[global_index];
    if ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') || byte == '-' || byte == '/') {
      req.pathname[pathname_index] = byte;
    }
    if (byte == '?') {
      stage = 2;
    }
    if (byte == ' ') {
      stage = 3;
    }
    pathname_index++;
    global_index++;
  }
  req.pathname[pathname_index] = '\0';
  if (stage == 1) {
    req.status = 414;
    return req;
  }

  while (stage == 2 && search_index < sizeof(req.search) - 1 && global_index < sizeof(*buffer)) {
    char byte = (*buffer)[global_index];
    if ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') || (byte >= '0' && byte <= '9') || byte == '=' ||
        byte == '&' || byte == '%') {
      req.search[search_index] = byte;
    }
    if (byte == ' ') {
      stage = 3;
    }
    search_index++;
    global_index++;
  }
  if (stage == 2) {
    req.status = 414;
    return req;
  }

  req.status = 0;
  return req;
}

#include "request.h"
#include <arpa/inet.h>

Request request(char (*buffer)[2048], ssize_t length) {
  struct Request req = {.method = {0}, .pathname = {0}, .search = {0}, .protocol = {0}, .headers = {0}, .status = 0};

  unsigned int stage = 0;
  unsigned int global_index = 0;

  unsigned int method_index = 0;
  while (stage == 0 && method_index < sizeof(req.method) - 1 && global_index < length) {
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

  unsigned int pathname_index = 0;
  while (stage == 1 && pathname_index < sizeof(req.pathname) - 1 && global_index < length) {
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

  unsigned int search_index = 0;
  while (stage == 2 && search_index < sizeof(req.search) - 1 && global_index < length) {
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
  req.search[search_index] = '\0';
  if (stage == 2) {
    req.status = 414;
    return req;
  }

  unsigned int protocol_index = 0;
  while ((stage == 3 || stage == 4) && protocol_index < sizeof(req.protocol) - 1 && global_index < length) {
    char byte = (*buffer)[global_index];
    if (byte >= 'A' && byte <= 'Z') {
      req.protocol[protocol_index] = byte + 32;
    }
    if ((byte >= '0' && byte <= '9') || byte == '.' || byte == '/') {
      req.protocol[protocol_index] = byte;
    }
    if (byte == '\r') {
      stage = 4;
    }
    if (byte == '\n') {
      stage = 5;
    }
    protocol_index++;
    global_index++;
  }
  req.protocol[protocol_index] = '\0';
  if (stage == 3) {
    req.status = 505;
    return req;
  }
  if (stage == 4) {
    req.status = 400;
    return req;
  }

  int headers_key = 1;
  unsigned int headers_index = 0;
  while ((stage >= 5 && stage <= 9) && headers_index < sizeof(req.headers) - 1 && global_index < length) {
    char byte = (*buffer)[global_index];
    if (headers_key && byte >= 'A' && byte <= 'Z') {
      req.headers[headers_index] = byte + 32;
    } else {
      req.headers[headers_index] = byte;
    }
    if (byte == ':') {
      headers_key = 0;
    }
    if (byte == '\r' || byte == '\n') {
      headers_key = 1;
      stage += 1;
    } else {
      stage = 5;
    }
    headers_index++;
    global_index++;
  }
  req.headers[headers_index] = '\0';
  if (stage >= 5 && stage <= 7) {
    req.status = 431;
    return req;
  }
  if (stage != 9) {
    req.status = 400;
    return req;
  }

  return req;
}

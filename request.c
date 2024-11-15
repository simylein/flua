#include "request.h"
#include <arpa/inet.h>
#include <string.h>

Request request(char (*buffer)[8192], ssize_t length) {
  struct Request req = {.method = {0}, .pathname = {0}, .search = {0}, .protocol = {0}, .header = {0}, .status = 0};

  int stage = 0;
  int global_index = 0;

  int method_index = 0;
  while (stage == 0 && method_index < (int)sizeof(req.method) - 1 && global_index < length) {
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

  int pathname_index = 0;
  while (stage == 1 && pathname_index < (int)sizeof(req.pathname) - 1 && global_index < length) {
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

  int search_index = 0;
  while (stage == 2 && search_index < (int)sizeof(req.search) - 1 && global_index < length) {
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

  int protocol_index = 0;
  while ((stage == 3 || stage == 4) && protocol_index < (int)sizeof(req.protocol) - 1 && global_index < length) {
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

  int header_key = 1;
  int header_index = 0;
  while ((stage >= 5 && stage <= 8) && header_index < (int)sizeof(req.header) - 1 && global_index < length) {
    char byte = (*buffer)[global_index];
    if (header_key && byte >= 'A' && byte <= 'Z') {
      req.header[header_index] = byte + 32;
    } else {
      req.header[header_index] = byte;
    }
    if (byte == ':') {
      header_key = 0;
    }
    if (byte == '\r' || byte == '\n') {
      header_key = 1;
      stage += 1;
    } else {
      stage = 5;
    }
    header_index++;
    global_index++;
  }
  req.header[header_index] = '\0';
  if (stage >= 5 && stage <= 7) {
    req.status = 431;
    return req;
  }
  if (stage == 8) {
    req.status = 400;
    return req;
  }

  int body_index = (int)length - global_index;
  if (body_index > (int)sizeof(req.body) - 1) {
    req.status = 413;
    return req;
  }

  memcpy(req.body, &(*buffer)[global_index], body_index);
  req.body[body_index] = '\0';

  return req;
}

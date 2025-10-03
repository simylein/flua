#pragma once

#include "file.h"

extern file_t page_home;
extern file_t page_robots;
extern file_t page_security;
extern file_t page_flight;
extern file_t page_flight_self;
extern file_t page_settings;
extern file_t page_signin;
extern file_t page_signup;

extern file_t page_bad_request;
extern file_t page_unauthorized;
extern file_t page_forbidden;
extern file_t page_not_found;
extern file_t page_method_not_allowed;
extern file_t page_uri_too_long;
extern file_t page_request_header_fields_too_large;
extern file_t page_internal_server_error;
extern file_t page_http_version_not_supported;

void page_init(void);
void page_close(void);
void page_free(void);

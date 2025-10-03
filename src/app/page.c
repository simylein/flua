#include "../lib/logger.h"
#include "file.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

file_t page_home = {.path = "./src/app/pages/home.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_robots = {.path = "./src/app/pages/robots.txt", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_security = {.path = "./src/app/pages/security.txt", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_flight = {.path = "./src/app/pages/flight.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_flight_self = {.path = "./src/app/pages/flight.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_settings = {.path = "./src/app/pages/settings.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_signin = {.path = "./src/app/pages/signin.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_signup = {.path = "./src/app/pages/signup.html", .lock = PTHREAD_RWLOCK_INITIALIZER};

file_t page_bad_request = {.path = "./src/app/errors/400.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_unauthorized = {.path = "./src/app/errors/401.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_forbidden = {.path = "./src/app/errors/403.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_not_found = {.path = "./src/app/errors/404.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_method_not_allowed = {.path = "./src/app/errors/405.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_uri_too_long = {.path = "./src/app/errors/414.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_request_header_fields_too_large = {.path = "./src/app/errors/431.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_internal_server_error = {.path = "./src/app/errors/500.html", .lock = PTHREAD_RWLOCK_INITIALIZER};
file_t page_http_version_not_supported = {.path = "./src/app/errors/505.html", .lock = PTHREAD_RWLOCK_INITIALIZER};

file_t *pages[] = {
		&page_home,
		&page_robots,
		&page_security,
		&page_flight,
		&page_flight_self,
		&page_settings,
		&page_signin,
		&page_signup,
		&page_bad_request,
		&page_unauthorized,
		&page_forbidden,
		&page_not_found,
		&page_method_not_allowed,
		&page_uri_too_long,
		&page_request_header_fields_too_large,
		&page_internal_server_error,
		&page_http_version_not_supported,
};

void page_init(void) {
	trace("initialising %zu pages\n", sizeof(pages) / sizeof(*pages));

	for (uint8_t index = 0; index < sizeof(pages) / sizeof(*pages); index++) {
		pages[index]->fd = -1;
		pages[index]->ptr = NULL;
		pages[index]->len = 0;
		pages[index]->modified = 0;
		pages[index]->hydrated = false;
	}
}

void page_close(void) {
	uint8_t closed = 0;

	for (uint8_t index = 0; index < sizeof(pages) / sizeof(*pages); index++) {
		if (pages[index]->fd != -1) {
			close(pages[index]->fd);
			pages[index]->fd = -1;
			closed += 1;
		}
	}

	trace("closed %hhu pages\n", closed);
}

void page_free(void) {
	uint8_t freed = 0;

	for (uint8_t index = 0; index < sizeof(pages) / sizeof(*pages); index++) {
		if (pages[index]->ptr != NULL) {
			free(pages[index]->ptr);
			pages[index]->ptr = NULL;
			pages[index]->len = 0;
			pages[index]->modified = 0;
			pages[index]->hydrated = false;
			freed += 1;
		}
	}

	trace("freed %hhu pages\n", freed);
}

#include "auth.h"
#include "bwt.h"
#include "file.h"
#include "flight.h"
#include "logger.h"
#include "parse.h"
#include "request.h"
#include "response.h"
#include "user.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

int match(request_t *request, const char *method, const char *pathname, bool *method_found, bool *pathname_found) {
	if (strcmp(request->pathname, pathname) == 0) {
		*pathname_found = true;

		if (strcmp(request->method, method) == 0) {
			*method_found = true;

			return 0;
		}
	}

	return -1;
}

void route(sqlite3 *database, request_t *request, response_t *response) {
	bool method_found = false;
	bool pathname_found = false;

	if (match(request, "get", "/", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("home.html", response);
	}

	if (match(request, "get", "/signin", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("signin.html", response);
	}

	if (match(request, "get", "/signup", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("signup.html", response);
	}

	if (match(request, "post", "/api/signin", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		char username[17];
		char password[65];
		if (parse_credentials(&username, &password, request) == -1) {
			debug("failed to parse username and password\n");
			response->status = 400;
			goto respond;
		};

		if (validate_credentials(&username, &password) == -1) {
			debug("failed to validate username and password\n");
			response->status = 400;
			goto respond;
		}

		create_signin(database, username, password, response);
	}

	if (match(request, "post", "/api/signup", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		char username[17];
		char password[65];
		if (parse_credentials(&username, &password, request) == -1) {
			debug("failed to parse username and password\n");
			response->status = 400;
			goto respond;
		};

		if (validate_credentials(&username, &password) == -1) {
			debug("failed to validate username and password\n");
			response->status = 400;
			goto respond;
		}

		create_signup(database, username, password, response);
	}

	if (match(request, "get", "/api/year", &method_found, &pathname_found) == 0) {
		const char *cookie = find_header(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (verify_bwt(cookie, &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		char name[17];
		if (sscanf(request->search, "user=%16s", name) != 1) {
			response->status = 400;
			goto respond;
		}

		struct user_t user;
		int status = find_user_by_name(database, name, &user);
		if (status != 0) {
			response->status = status;
			goto respond;
		}

		if (user.public == false && memcmp(bwt.id, user.id, sizeof(user.id)) != 0) {
			response->status = 403;
			goto respond;
		}

		find_years(database, &user.id, response);
	}

	if (match(request, "get", "/api/flight", &method_found, &pathname_found) == 0) {
		const char *cookie = find_header(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (verify_bwt(cookie, &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		char name[17];
		char year[5];
		if (sscanf(request->search, "user=%16[^&]&year=%4s", name, year) != 2) {
			response->status = 400;
			goto respond;
		}

		struct user_t user;
		int status = find_user_by_name(database, name, &user);
		if (status != 0) {
			response->status = status;
			goto respond;
		}

		if (user.public == false && memcmp(bwt.id, user.id, sizeof(user.id)) != 0) {
			response->status = 403;
			goto respond;
		}

		find_flights(database, &user.id, year, response);
	}

	if (match(request, "post", "/api/flight", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = find_header(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (verify_bwt(cookie, &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		char hash[33];
		uint64_t starts_at;
		uint64_t ends_at;

		request->body[request->body_len] = '\0';

		if (sscanf(request->body, "hash=%32s&starts_at=%lld&ends_at=%lld", hash, &starts_at, &ends_at) != 3) {
			response->status = 400;
			goto respond;
		}

		create_flight(database, &bwt, hash, starts_at, ends_at, response);
	}

	if (pathname_found == 0 && request->pathname_len >= 5 && request->pathname_len <= 17) {
		for (size_t index = 0; index < request->pathname_len; index++) {
			if (index != 0 && request->pathname[index] != '-' && (request->pathname[index] < 'a' || request->pathname[index] > 'z')) {
				goto respond;
			}
		}

		pathname_found = 1;
		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			const char *cookie = find_header(request, "cookie:");
			if (cookie == NULL) {
				response->status = 307;
				append_header(response, "location:/signin\r\n");
				goto respond;
			}

			struct bwt_t bwt;
			if (verify_bwt(cookie, &bwt) == -1) {
				response->status = 401;
				goto respond;
			}

			struct user_t user;
			int status = find_user_by_name(database, &request->pathname[1], &user);
			if (status != 0) {
				response->status = status;
				goto respond;
			}

			if (user.public == false && memcmp(bwt.id, user.id, sizeof(user.id)) != 0) {
				response->status = 403;
				goto respond;
			}

			file("flight.html", response);
		}
	}

respond:

	if (pathname_found == false && method_found == false) {
		response->status = 404;
	}
	if (pathname_found == true && method_found == false) {
		response->status = 405;
	}

	if (memcmp(request->pathname, "/api/", 5) == 0) {
		return;
	}

	if (response->status == 400) {
		file("400.html", response);
	}
	if (response->status == 401) {
		file("401.html", response);
	}
	if (response->status == 403) {
		file("403.html", response);
	}
	if (response->status == 404) {
		file("404.html", response);
	}
	if (response->status == 405) {
		file("405.html", response);
	}
	if (response->status == 500) {
		file("500.html", response);
	}
}

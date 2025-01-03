#include "auth.h"
#include "bwt.h"
#include "file.h"
#include "flight.h"
#include "logger.h"
#include "parse.h"
#include "request.h"
#include "response.h"
#include "user.h"
#include "utils.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

file_t home = {.ptr = NULL, .len = 0};
file_t robots = {.ptr = NULL, .len = 0};
file_t security = {.ptr = NULL, .len = 0};
file_t signin = {.ptr = NULL, .len = 0};
file_t signup = {.ptr = NULL, .len = 0};
file_t flight = {.ptr = NULL, .len = 0};

file_t bad_request = {.ptr = NULL, .len = 0};
file_t unauthorized = {.ptr = NULL, .len = 0};
file_t forbidden = {.ptr = NULL, .len = 0};
file_t not_found = {.ptr = NULL, .len = 0};
file_t method_not_allowed = {.ptr = NULL, .len = 0};
file_t internal_server_error = {.ptr = NULL, .len = 0};

int match(request_t *request, const char *method, const char *pathname, bool *method_found, bool *pathname_found) {
	if (request->pathname_len == strlen(pathname) && memcmp(request->pathname, pathname, request->pathname_len) == 0) {
		*pathname_found = true;

		if (request->method_len == strlen(method) && memcmp(request->method, method, request->method_len) == 0) {
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

		const char *cookie = find_header(request, "cookie:");
		if (cookie != NULL) {
			struct bwt_t bwt;
			if (verify_bwt(cookie, request->header_len - (size_t)(cookie - (const char *)request->header), &bwt) == 0) {
				struct user_t user;
				if (find_user_by_id(database, &bwt.id, &user) == 0) {
					debug("redirecting to location %s\n", user.username);
					response->status = 307;
					append_header(response, "location:/%s\r\n", user.username);
					goto respond;
				}
			}
		}

		file("home.html", &home, response);
	}

	if (match(request, "get", "/robots.txt", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("robots.txt", &robots, response);
	}

	if (match(request, "get", "/security.txt", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("security.txt", &security, response);
	}

	if (match(request, "get", "/signin", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("signin.html", &signin, response);
	}

	if (match(request, "get", "/signup", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("signup.html", &signup, response);
	}

	if (match(request, "get", "/api/me", &method_found, &pathname_found) == 0) {
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
		if (verify_bwt(cookie, request->header_len - (size_t)(cookie - (const char *)request->header), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		find_user(database, &bwt, response);
	}

	if (match(request, "delete", "/api/me", &method_found, &pathname_found) == 0) {
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
		if (verify_bwt(cookie, request->header_len - (size_t)(cookie - (const char *)request->header), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		delete_user(database, &bwt, response);
	}

	if (match(request, "post", "/api/signin", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		char *username;
		uint8_t username_len;
		char *password;
		uint8_t password_len;
		if (parse_credentials(&username, &username_len, &password, &password_len, request) == -1) {
			debug("failed to parse credentials\n");
			response->status = 400;
			goto respond;
		};

		if (validate_credentials(&username, &username_len, &password, &password_len) == -1) {
			debug("failed to validate credentials\n");
			response->status = 400;
			goto respond;
		}

		create_signin(database, username, username_len, password, password_len, response);
	}

	if (match(request, "post", "/api/signup", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		char *username;
		uint8_t username_len;
		char *password;
		uint8_t password_len;
		if (parse_credentials(&username, &username_len, &password, &password_len, request) == -1) {
			debug("failed to parse credentials\n");
			response->status = 400;
			goto respond;
		};

		if (validate_credentials(&username, &username_len, &password, &password_len) == -1) {
			debug("failed to validate credentials\n");
			response->status = 400;
			goto respond;
		}

		create_signup(database, username, username_len, password, password_len, response);
	}

	if (match(request, "get", "/api/year", &method_found, &pathname_found) == 0) {
		char *name;
		size_t name_len;
		if (strnfind(request->search, request->search_len, "user=", "", &name, &name_len, 16) == -1) {
			response->status = 400;
			goto respond;
		}

		struct user_t user;
		int status = find_user_by_name(database, name, name_len, &user);
		if (status != 0) {
			response->status = status;
			goto respond;
		}

		if (user.public == false) {
			const char *cookie = find_header(request, "cookie:");
			if (cookie == NULL) {
				response->status = 401;
				goto respond;
			}

			struct bwt_t bwt;
			if (verify_bwt(cookie, request->header_len - (size_t)(cookie - (const char *)request->header), &bwt) == -1) {
				response->status = 401;
				goto respond;
			}

			if (memcmp(bwt.id, user.id, sizeof(user.id)) != 0) {
				response->status = 403;
				goto respond;
			}
		}

		find_years(database, &user.id, response);
	}

	if (match(request, "get", "/api/flight", &method_found, &pathname_found) == 0) {
		char *name;
		size_t name_len;
		if (strnfind(request->search, request->search_len, "user=", "&", &name, &name_len, 16) == -1) {
			response->status = 400;
			goto respond;
		}

		char *year;
		size_t year_len;
		if (strnfind(request->search, request->search_len, "&year=", "", &year, &year_len, 4) == -1) {
			response->status = 400;
			goto respond;
		}

		struct user_t user;
		int status = find_user_by_name(database, name, name_len, &user);
		if (status != 0) {
			response->status = status;
			goto respond;
		}

		if (user.public == false) {
			const char *cookie = find_header(request, "cookie:");
			if (cookie == NULL) {
				response->status = 401;
				goto respond;
			}

			struct bwt_t bwt;
			if (verify_bwt(cookie, request->header_len - (size_t)(cookie - (const char *)request->header), &bwt) == -1) {
				response->status = 401;
				goto respond;
			}

			if (memcmp(bwt.id, user.id, sizeof(user.id)) != 0) {
				response->status = 403;
				goto respond;
			}
		}

		find_flights(database, &user.id, year, year_len, response);
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
		if (verify_bwt(cookie, request->header_len - (size_t)(cookie - (const char *)request->header), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		struct flight_t new_flight;
		if (parse_flight(&new_flight, request) == -1) {
			debug("failed to parse flight\n");
			response->status = 400;
			goto respond;
		};

		if (validate_flight(&new_flight) == -1) {
			debug("failed to validate flight\n");
			response->status = 400;
			goto respond;
		};

		create_flight(database, &bwt, &new_flight, response);
	}

	if (pathname_found == 0 && request->pathname_len >= 5 && request->pathname_len <= 17) {
		for (size_t index = 0; index < request->pathname_len; index++) {
			if (index != 0 && request->pathname[index] != '-' && (request->pathname[index] < 'a' || request->pathname[index] > 'z')) {
				goto respond;
			}
		}

		pathname_found = 1;
		if (request->method_len == 3 && memcmp(request->method, "get", request->method_len) == 0) {
			method_found = 1;

			struct user_t user;
			int status = find_user_by_name(database, &request->pathname[1], request->pathname_len - 1, &user);
			if (status != 0) {
				response->status = status;
				goto respond;
			}

			const char *cookie = find_header(request, "cookie:");
			if (user.public == false && cookie == NULL) {
				debug("redirecting to location signin\n");
				response->status = 307;
				append_header(response, "location:/signin\r\n");
				append_header(response, "set-cookie:memo=%.*s\r\n", (int)request->pathname_len, request->pathname);
				goto respond;
			}

			if (cookie != NULL) {
				struct bwt_t bwt;
				if (verify_bwt(cookie, request->header_len - (size_t)(cookie - (const char *)request->header), &bwt) == -1) {
					response->status = 401;
					goto respond;
				}

				if (user.public == false && memcmp(bwt.id, user.id, sizeof(user.id)) != 0) {
					response->status = 403;
					goto respond;
				}
			}

			file("flight.html", &flight, response);
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
		file("400.html", &bad_request, response);
	}
	if (response->status == 401) {
		file("401.html", &unauthorized, response);
	}
	if (response->status == 403) {
		file("403.html", &forbidden, response);
	}
	if (response->status == 404) {
		file("404.html", &not_found, response);
	}
	if (response->status == 405) {
		file("405.html", &method_not_allowed, response);
	}
	if (response->status == 500) {
		file("500.html", &internal_server_error, response);
	}
}

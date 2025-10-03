#include "../app/cull.h"
#include "../app/page.h"
#include "../app/serve.h"
#include "../lib/bwt.h"
#include "../lib/logger.h"
#include "../lib/request.h"
#include "../lib/response.h"
#include "auth.h"
#include "flight.h"
#include "friend.h"
#include "parse.h"
#include "user.h"
#include "utils.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

int match(request_t *request, const char *method, const char *pathname, bool *method_found, bool *pathname_found) {
	if (request->pathname.len == strlen(pathname) && memcmp(request->pathname.ptr, pathname, request->pathname.len) == 0) {
		*pathname_found = true;

		if (strcmp(method, "get") == 0 && request->method.len == 4 &&
				memcmp(request->method.ptr, "head", request->method.len) == 0) {
			*method_found = true;

			return 0;
		}

		if (request->method.len == strlen(method) && memcmp(request->method.ptr, method, request->method.len) == 0) {
			*method_found = true;

			return 0;
		}
	}

	return -1;
}

void route(sqlite3 *database, request_t *request, response_t *response) {
	bool method_found = false;
	bool pathname_found = false;

	if (response->status != 0) {
		goto respond;
	}

	if (match(request, "get", "/", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = header_find(request, "cookie:");
		if (cookie != NULL) {
			struct bwt_t bwt;
			if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == 0) {
				struct user_t user;
				if (find_user_by_id(database, &bwt.id, &user) == 0) {
					debug("redirecting to location %s\n", user.username);
					response->status = 307;
					header_write(response, "location:/%s\r\n", user.username);
					goto respond;
				}
			}
		}

		serve(&page_home, NULL, response);
	}

	if (match(request, "get", "/robots.txt", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		serve(&page_robots, NULL, response);
	}

	if (match(request, "get", "/security.txt", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		serve(&page_security, NULL, response);
	}

	if (match(request, "get", "/signin", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		serve(&page_signin, NULL, response);
	}

	if (match(request, "get", "/signup", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		serve(&page_signup, NULL, response);
	}

	if (match(request, "get", "/settings", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = header_find(request, "cookie:");
		if (cookie == NULL) {
			debug("redirecting to location signin\n");
			response->status = 307;
			header_write(response, "location:/signin\r\n");
			header_write(response, "set-cookie:memo=%.*s\r\n", (int)request->pathname.len, request->pathname.ptr);
			goto respond;
		}

		struct bwt_t bwt;
		if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		serve(&page_settings, NULL, response);
	}

	if (match(request, "get", "/api/me", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = header_find(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		find_user(database, &bwt, response);
	}

	if (match(request, "patch", "/api/me", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = header_find(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		struct user_t new_user;
		if (parse_user(&new_user, request) == -1) {
			debug("failed to parse user\n");
			response->status = 400;
			goto respond;
		};

		if (validate_user(&new_user) == -1) {
			debug("failed to validate user\n");
			response->status = 400;
			goto respond;
		};

		update_user(database, &bwt, &new_user, response);
	}

	if (match(request, "delete", "/api/me", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = header_find(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		delete_user(database, &bwt, response);
	}

	if (match(request, "post", "/api/signin", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
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
		if (request->search.len != 0) {
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
		const char *name;
		size_t name_len;
		if (strnfind(request->search.ptr, request->search.len, "user=", "", &name, &name_len, 16) == -1) {
			response->status = 400;
			goto respond;
		}

		struct user_t user;
		uint16_t user_status = find_user_by_name(database, name, name_len, &user);
		if (user_status != 0) {
			response->status = user_status;
			goto respond;
		}

		if (user.visibility == private || user.visibility == friends) {
			const char *cookie = header_find(request, "cookie:");
			if (cookie == NULL) {
				response->status = 401;
				goto respond;
			}

			struct bwt_t bwt;
			if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
				response->status = 401;
				goto respond;
			}

			bool self = memcmp(bwt.id, user.id, sizeof(user.id)) == 0;
			if (user.visibility == private && self == false) {
				response->status = 403;
				goto respond;
			}

			struct friend_t friend;
			uint16_t friend_status = find_friend_by_user_id(database, &bwt, &user, &friend);
			if (user.visibility == friends && self == false && friend_status != 0) {
				response->status = friend_status == 404 ? 403 : friend_status;
				goto respond;
			}
		}

		find_years(database, &user.id, response);
	}

	if (match(request, "get", "/api/flight", &method_found, &pathname_found) == 0) {
		const char *name;
		size_t name_len;
		if (strnfind(request->search.ptr, request->search.len, "user=", "&", &name, &name_len, 16) == -1) {
			response->status = 400;
			goto respond;
		}

		const char *year;
		size_t year_len;
		if (strnfind(request->search.ptr, request->search.len, "&year=", "", &year, &year_len, 4) == -1) {
			response->status = 400;
			goto respond;
		}

		struct user_t user;
		uint16_t user_status = find_user_by_name(database, name, name_len, &user);
		if (user_status != 0) {
			response->status = user_status;
			goto respond;
		}

		if (user.visibility == private || user.visibility == friends) {
			const char *cookie = header_find(request, "cookie:");
			if (cookie == NULL) {
				response->status = 401;
				goto respond;
			}

			struct bwt_t bwt;
			if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
				response->status = 401;
				goto respond;
			}

			bool self = memcmp(bwt.id, user.id, sizeof(user.id)) == 0;
			if (user.visibility == private && self == false) {
				response->status = 403;
				goto respond;
			}

			struct friend_t friend;
			uint16_t friend_status = find_friend_by_user_id(database, &bwt, &user, &friend);
			if (user.visibility == friends && self == false && friend_status != 0) {
				response->status = friend_status == 404 ? 403 : friend_status;
				goto respond;
			}
		}

		find_flights(database, &user.id, year, year_len, response);
	}

	if (match(request, "post", "/api/flight", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = header_find(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
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

	if (match(request, "delete", "/api/flight", &method_found, &pathname_found) == 0) {
		if (request->search.len != 0) {
			response->status = 400;
			goto respond;
		}

		const char *cookie = header_find(request, "cookie:");
		if (cookie == NULL) {
			response->status = 401;
			goto respond;
		}

		struct bwt_t bwt;
		if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
			response->status = 401;
			goto respond;
		}

		delete_flights(database, &bwt, response);
	}

	if (pathname_found == 0 && request->pathname.len >= 5 && request->pathname.len <= 17) {
		for (size_t index = 0; index < request->pathname.len; index++) {
			if (index != 0 && request->pathname.ptr[index] != '-' &&
					(request->pathname.ptr[index] < 'a' || request->pathname.ptr[index] > 'z')) {
				goto respond;
			}
		}

		pathname_found = 1;
		if ((request->method.len == 3 && memcmp(request->method.ptr, "get", request->method.len) == 0) ||
				(request->method.len == 4 && memcmp(request->method.ptr, "head", request->method.len) == 0)) {
			method_found = 1;

			struct user_t user;
			uint16_t user_status = find_user_by_name(database, &request->pathname.ptr[1], request->pathname.len - 1, &user);
			if (user_status != 0) {
				response->status = user_status;
				goto respond;
			}

			const char *cookie = header_find(request, "cookie:");
			if ((user.visibility == private || user.visibility == friends) && cookie == NULL) {
				debug("redirecting to location signin\n");
				response->status = 307;
				header_write(response, "location:/signin\r\n");
				header_write(response, "set-cookie:memo=%.*s\r\n", (int)request->pathname.len, request->pathname.ptr);
				goto respond;
			}

			bool self = false;
			if (cookie != NULL) {
				struct bwt_t bwt;
				if (bwt_verify(cookie, request->header.len - (size_t)(cookie - (const char *)request->header.ptr), &bwt) == -1) {
					response->status = 401;
					goto respond;
				}

				self = memcmp(bwt.id, user.id, sizeof(user.id)) == 0;
				if (user.visibility == private && self == false) {
					response->status = 403;
					goto respond;
				}

				struct friend_t friend;
				uint16_t friend_status = find_friend_by_user_id(database, &bwt, &user, &friend);
				if (user.visibility == friends && self == false && friend_status != 0) {
					response->status = friend_status == 404 ? 403 : friend_status;
					goto respond;
				}
			}

			if (self == true) {
				serve(&page_flight_self, NULL, response);
			} else {
				serve(&page_flight, cull_flight, response);
			}
		}
	}

respond:

	if (response->status == 0 && pathname_found == false && method_found == false) {
		response->status = 404;
	}
	if (response->status == 0 && pathname_found == true && method_found == false) {
		response->status = 405;
	}

	if (memcmp(request->pathname.ptr, "/api/", 5) == 0) {
		return;
	}

	if (response->status == 400) {
		serve(&page_bad_request, NULL, response);
	}
	if (response->status == 401) {
		serve(&page_unauthorized, NULL, response);
	}
	if (response->status == 403) {
		serve(&page_forbidden, NULL, response);
	}
	if (response->status == 404) {
		serve(&page_not_found, NULL, response);
	}
	if (response->status == 405) {
		serve(&page_method_not_allowed, NULL, response);
	}
	if (response->status == 414) {
		serve(&page_uri_too_long, NULL, response);
	}
	if (response->status == 431) {
		serve(&page_request_header_fields_too_large, NULL, response);
	}
	if (response->status == 500) {
		serve(&page_internal_server_error, NULL, response);
	}
	if (response->status == 505) {
		serve(&page_http_version_not_supported, NULL, response);
	}
}

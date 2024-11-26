#include "auth.h"
#include "file.h"
#include "flight.h"
#include "request.h"
#include "response.h"
#include <stdlib.h>
#include <string.h>

int match(Request *request, const char *method, const char *pathname, int *method_found, int *pathname_found) {
	if (strcmp(request->pathname, pathname) == 0) {
		*pathname_found = 1;

		if (strcmp(request->method, method) == 0) {
			*method_found = 1;

			return 0;
		}
	}

	return -1;
}

void route(Request *request, Response *response) {
	int method_found = 0;
	int pathname_found = 0;

	if (match(request, "get", "/", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("home.html", response);
	}

	if (match(request, "get", "/home.css", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("home.css", response);
	}

	if (match(request, "get", "/error.css", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("error.css", response);
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

	if (match(request, "get", "/auth.css", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("auth.css", response);
	}

	if (match(request, "get", "/flight.css", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("flight.css", response);
	}

	if (match(request, "get", "/flight.js", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("flight.js", response);
	}

	if (match(request, "get", "/upload.js", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("upload.js", response);
	}

	if (match(request, "get", "/notifications.css", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("notifications.css", response);
	}

	if (match(request, "get", "/notifications.js", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		file("notifications.js", response);
	}

	if (match(request, "post", "/api/signin", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		char username[17];
		char password[65];

		request->body[request->body_len] = '\0';

		if (sscanf(request->body, "username=%16[^&]&password=%64s", username, password) != 2) {
			response->status = 400;
			goto respond;
		}

		if (strlen(username) < 4 || strlen(password) < 8) {
			response->status = 400;
			goto respond;
		}

		create_signin(username, password, response);
	}

	if (match(request, "post", "/api/signup", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		char username[17];
		char password[65];

		request->body[request->body_len] = '\0';

		if (sscanf(request->body, "username=%16[^&]&password=%64s", username, password) != 2) {
			response->status = 400;
			goto respond;
		}

		if (strlen(username) < 4 || strlen(password) < 8) {
			response->status = 400;
			goto respond;
		}

		create_signup(username, password, response);
	}

	if (match(request, "get", "/api/flight", &method_found, &pathname_found) == 0) {
		char user_id[33];
		if (authenticate(request, &user_id) == -1) {
			response->status = 401;
			goto respond;
		}

		char year[5];
		if (sscanf(request->search, "year=%4s", year) == 1) {
			find_flights(user_id, year, response);
			goto respond;
		}

		find_flight_years(user_id, response);
	}

	if (match(request, "post", "/api/flight", &method_found, &pathname_found) == 0) {
		if (request->search_len != 0) {
			response->status = 400;
			goto respond;
		}

		char user_id[33];
		if (authenticate(request, &user_id) == -1) {
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

		create_flight(user_id, hash, starts_at, ends_at, response);
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

			file("flight.html", response);
		}
	}

respond:

	if (pathname_found == 0 && method_found == 0) {
		response->status = 404;
	}
	if (pathname_found == 1 && method_found == 0) {
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

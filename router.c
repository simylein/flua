#include "auth.h"
#include "file.h"
#include "flight.h"
#include "request.h"
#include "response.h"
#include <stdlib.h>
#include <string.h>

void route(Request *request, Response *response) {
	int method_found = 0;
	int pathname_found = 0;

	if (strcmp(request->pathname, "/") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			response->status = 200;
			file("home.html", response);
		}
	}

	if (strcmp(request->pathname, "/home.css") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			response->status = 200;
			file("home.css", response);
		}
	}

	if (strcmp(request->pathname, "/error.css") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			response->status = 200;
			file("error.css", response);
		}
	}

	if (strcmp(request->pathname, "/signin") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			response->status = 200;
			file("signin.html", response);
		}
	}

	if (strcmp(request->pathname, "/signup") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			response->status = 200;
			file("signup.html", response);
		}
	}

	if (strcmp(request->pathname, "/flight.css") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			response->status = 200;
			file("flight.css", response);
		}
	}

	if (strcmp(request->pathname, "/flight.js") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			response->status = 200;
			file("flight.js", response);
		}
	}

	if (strcmp(request->pathname, "/api/signin") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "post") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			char username[17];
			char password[65];

			if (sscanf(request->body, "username=%16[^&]&password=%64s", username, password) != 2) {
				response->status = 400;
				goto respond;
			}

			if (strlen(username) < 4 && strlen(password) < 8) {
				response->status = 400;
				goto respond;
			}

			response->status = 201;
			create_signin(username, password, response);
		}
	}

	if (strcmp(request->pathname, "/api/signup") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "post") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			char username[17];
			char password[65];

			if (sscanf(request->body, "username=%16[^&]&password=%64s", username, password) != 2) {
				response->status = 400;
				goto respond;
			}

			if (strlen(username) < 4 && strlen(password) < 8) {
				response->status = 400;
				goto respond;
			}

			response->status = 201;
			create_signup(username, password, response);
		}
	}

	if (strcmp(request->pathname, "/api/flight") == 0) {
		pathname_found = 1;

		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			char user_id[33];
			if (authenticate(request, &user_id) == -1) {
				response->status = 401;
				goto respond;
			}

			char year[5];
			if (sscanf(request->search, "year=%4s", year) == 1) {
				response->status = 200;
				find_flights(user_id, year, response);
				goto respond;
			}

			response->status = 200;
			find_flight_years(user_id, response);
		}

		if (strcmp(request->method, "post") == 0) {
			method_found = 1;

			if (request->search_len != 0) {
				response->status = 400;
				goto respond;
			}

			char user_id[33];
			if (authenticate(request, &user_id) == -1) {
				response->status = 401;
				goto respond;
			}

			char hash[17];
			uint64_t starts_at;
			uint64_t ends_at;

			if (sscanf(request->body, "hash=%16[^&]&starts_at=%lld&ends_at=%lld", hash, &starts_at, &ends_at) != 3) {
				response->status = 400;
				goto respond;
			}

			response->status = 201;
			create_flight(user_id, hash, starts_at, ends_at, response);
		}
	}

	if (pathname_found == 0 && request->pathname_len >= 5 && request->pathname_len <= 17) {
		for (size_t index = 0; index < request->pathname_len; index++) {
			if (index != 0 && request->pathname[index] == '/') {
				goto respond;
			}
		}

		pathname_found = 1;
		if (strcmp(request->method, "get") == 0) {
			method_found = 1;

			response->status = 200;
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

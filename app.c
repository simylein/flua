#include "auth.h"
#include "file.h"
#include "flight.h"
#include "request.h"
#include "response.h"
#include <stdlib.h>
#include <string.h>

void handle(Request *request, Response *response) {
	int method_found = 0;
	int pathname_found = 0;

	if (strcmp(request->pathname, "/") == 0) {
		pathname_found = 1;
		if (strcmp(request->method, "get") == 0) {
			method_found = 1;
			if (strlen(request->search) == 0) {
				response->status = 200;
				file("home.html", response);
			} else {
				response->status = 400;
			}
		}
	}

	if (strcmp(request->pathname, "/signin") == 0) {
		pathname_found = 1;
		if (strcmp(request->method, "get") == 0) {
			method_found = 1;
			if (strlen(request->search) == 0) {
				response->status = 200;
				file("signin.html", response);
			} else {
				response->status = 400;
			}
		}
	}

	if (strcmp(request->pathname, "/signup") == 0) {
		pathname_found = 1;
		if (strcmp(request->method, "get") == 0) {
			method_found = 1;
			if (strlen(request->search) == 0) {
				response->status = 200;
				file("signup.html", response);
			} else {
				response->status = 400;
			}
		}
	}

	if (strcmp(request->pathname, "/api/signin") == 0) {
		pathname_found = 1;
		if (strcmp(request->method, "post") == 0) {
			method_found = 1;
			if (strlen(request->search) == 0) {
				char username[17];
				char password[65];
				if (sscanf(request->body, "username=%16[^&]&password=%64s", username, password) == 2) {
					if (strlen(username) >= 4 && strlen(password) >= 8) {
						response->status = 201;
						user_signin(username, password, response);
					} else {
						response->status = 400;
					}
				} else {
					response->status = 400;
				}
			} else {
				response->status = 400;
			}
		}
	}

	if (strcmp(request->pathname, "/api/signup") == 0) {
		pathname_found = 1;
		if (strcmp(request->method, "post") == 0) {
			method_found = 1;
			if (strlen(request->search) == 0) {
				char username[17];
				char password[65];
				if (sscanf(request->body, "username=%16[^&]&password=%64s", username, password) == 2) {
					if (strlen(username) >= 4 && strlen(password) >= 8) {
						response->status = 201;
						user_signup(username, password, response);
					} else {
						response->status = 400;
					}
				} else {
					response->status = 400;
				}
			} else {
				response->status = 400;
			}
		}
	}

	if (strcmp(request->pathname, "/api/flight") == 0) {
		pathname_found = 1;
		if (strcmp(request->method, "get") == 0) {
			method_found = 1;
			char year[5];
			// TODO: do not hardcode user id
			int user_id = 1;
			if (sscanf(request->search, "year=%4s", year) == 1) {
				response->status = 200;
				find_flights(year, user_id, response);
			} else {
				response->status = 200;
				find_flight_years(user_id, response);
			}
		}
	}

	if (pathname_found == 0 && method_found == 0) {
		response->status = 404;
	}
	if (pathname_found == 1 && method_found == 0) {
		response->status = 405;
	}

	const char *prefix = "/api/";
	if (memcmp(request->pathname, prefix, strlen(prefix)) == 0) {
		return;
	}
	if (response->status == 400) {
		file("400.html", response);
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

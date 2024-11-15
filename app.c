#include "request.h"
#include "response.h"
#include <stdio.h>
#include <string.h>

void handle(Request *req, Response *res) {
	res->status = 200;
	memcpy(res->header, req->header, strlen(req->header));
	memcpy(res->body, req->body, strlen(req->body));
}

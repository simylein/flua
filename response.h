typedef struct Response {
	int status;
	char header[1024];
	char body[6144];
} Response;

int response(char (*buffer)[8192], Response *res);

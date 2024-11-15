typedef struct Response {
  int status;
} Response;

int response(char (*buffer)[2048], Response *res);

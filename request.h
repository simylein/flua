typedef struct Request {
  char method[8];
  char pathname[64];
  char search[128];
  char protocol[16];
  char headers[1024];
  int status;
} Request;

Request request(char (*buffer)[2048]);

#include "logger.h"
#include <stdlib.h>
#include <string.h>

int port = 1681;
int backlog = 16;

char *database_file = "flua.sqlite";

int log_level = 4;
int log_requests = 1;
int log_responses = 1;

char *next_arg(int argc, char *argv[], int *ind) {
  (*ind)++;
  if (*ind < argc) {
    return argv[*ind];
  }
  return NULL;
}

int parse_int(char *arg, char *name, int min, int max, int *value) {
  if (arg == NULL) {
    error("please provide a value for %s\n", name);
    return 1;
  }

  int new_value = atoi(arg);
  if (new_value < min || new_value > max) {
    error("%s must be between %d and %d\n", name, min, max);
    return 1;
  }

  *value = new_value;
  return 0;
}

int parse_bool(char *arg, char *name, int *value) {
  if (arg == NULL) {
    error("please provide a value for %s\n", name);
    return 1;
  }

  if (strcmp(arg, "false") == 0) {
    *value = 0;
  } else if (strcmp(arg, "true") == 0) {
    *value = 1;
  } else {
    error("%s must be either true or false\n", name);
    return 1;
  }

  return 0;
}

int parse_str(char *arg, char *name, unsigned int min, unsigned int max, char **value) {
  if (arg == NULL) {
    error("please provide a value for %s\n", name);
    return 1;
  }

  int len = strlen(arg);
  if (len < min || len > max) {
    error("%s must be between %d and %d characters\n", name, min, max);
    return 1;
  }

  *value = arg;
  return 0;
}

int parse_log_level(char *arg, char *name, int *value) {
  if (arg == NULL) {
    error("please provide a value for %s\n", name);
    return 1;
  }

  int new_value;
  if (strcmp(arg, "panic") == 0) {
    new_value = 1;
  } else if (strcmp(arg, "error") == 0) {
    new_value = 2;
  } else if (strcmp(arg, "warn") == 0) {
    new_value = 3;
  } else if (strcmp(arg, "info") == 0) {
    new_value = 4;
  } else if (strcmp(arg, "debug") == 0) {
    new_value = 5;
  } else if (strcmp(arg, "trace") == 0) {
    new_value = 6;
  } else {
    error("%s must be one of trace debug info warn error panic\n", name);
    return 1;
  }

  *value = new_value;
  return 0;
}

int configure(int argc, char *argv[]) {
  int errors = 0;

  for (int ind = 1; ind < argc; ind++) {
    char *flag = argv[ind];
    if (strcmp(flag, "--port") == 0 || strcmp(flag, "-p") == 0) {
      char *arg = next_arg(argc, argv, &ind);
      errors += parse_int(arg, "port", 1, 65535, &port);
    } else if (strcmp(flag, "--backlog") == 0 || strcmp(flag, "-b") == 0) {
      char *arg = next_arg(argc, argv, &ind);
      errors += parse_int(arg, "backlog", 2, 256, &backlog);
    } else if (strcmp(flag, "--database-file") == 0 || strcmp(flag, "-df") == 0) {
      char *arg = next_arg(argc, argv, &ind);
      errors += parse_str(arg, "database file", 4, 64, &database_file);
    } else if (strcmp(flag, "--log-level") == 0 || strcmp(flag, "-ll") == 0) {
      char *arg = next_arg(argc, argv, &ind);
      errors += parse_log_level(arg, "log level", &log_level);
    } else if (strcmp(flag, "--log-requests") == 0 || strcmp(flag, "-lrq") == 0) {
      char *arg = next_arg(argc, argv, &ind);
      errors += parse_bool(arg, "log requests", &log_requests);
    } else if (strcmp(flag, "--log-responses") == 0 || strcmp(flag, "-lrs") == 0) {
      char *arg = next_arg(argc, argv, &ind);
      errors += parse_bool(arg, "log responses", &log_responses);
    } else {
      errors++;
      error("unknown argument %s\n", flag);
    }
  }

  return errors;
}

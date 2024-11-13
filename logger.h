void req(const char *message, ...) __attribute__((format(printf, 1, 2)));
void res(const char *message, ...) __attribute__((format(printf, 1, 2)));

void trace(const char *message, ...) __attribute__((format(printf, 1, 2)));
void debug(const char *message, ...) __attribute__((format(printf, 1, 2)));
void info(const char *message, ...) __attribute__((format(printf, 1, 2)));
void warn(const char *message, ...) __attribute__((format(printf, 1, 2)));
void error(const char *message, ...) __attribute__((format(printf, 1, 2)));
void fatal(const char *message, ...) __attribute__((format(printf, 1, 2)));

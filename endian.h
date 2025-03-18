#include <arpa/inet.h>
#include <stdint.h>

#ifndef htonll
uint64_t htonll(uint64_t value);
#endif

#ifndef ntohll
uint64_t ntohll(uint64_t value);
#endif

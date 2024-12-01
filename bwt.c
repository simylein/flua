#include "bwt.h"
#include "config.h"
#include "endian.h"
#include "format.h"
#include "logger.h"
#include "request.h"
#include "string.h"
#include "time.h"
#include <stdint.h>

// TODO: implement actual signing
int sign_bwt(char (*buffer)[65], const uint8_t *id, const size_t id_len) {
	if (bin_to_hex(*buffer, id_len * 2 + 1, id, id_len) == -1) {
		error("failed to convert id to hex\n");
		return -1;
	}

	const time_t iat = time(NULL);
	const uint64_t n_iat = htonll(iat);
	if (bin_to_hex(*buffer + id_len * 2, sizeof(n_iat) * 2 + 1, &n_iat, sizeof(n_iat)) == -1) {
		error("failed to convert iat to hex\n");
		return -1;
	}

	const time_t exp = iat + bwt_ttl;
	const uint64_t n_exp = htonll(exp);
	if (bin_to_hex(*buffer + id_len * 2 + sizeof(n_iat) * 2, sizeof(n_exp) * 2 + 1, &n_exp, sizeof(n_exp)) == -1) {
		error("failed to convert exp to hex\n");
		return -1;
	}

	return 0;
}

// TODO: implement actual verification
int verify_bwt(const char *cookie, bwt_t *bwt) {
	char buffer[65];
	if (sscanf(cookie, "auth=%64s\r\n", buffer) != 1) {
		warn("no auth value in cookie header\n");
		return -1;
	}

	if (hex_to_bin(bwt->id, sizeof(bwt->id), buffer, sizeof(bwt->id) * 2) == -1) {
		error("failed to convert id to binary\n");
		return -1;
	}

	uint64_t n_iat;
	if (hex_to_bin(&n_iat, sizeof(n_iat), &buffer[sizeof(bwt->id) * 2], sizeof(n_iat) * 2) == -1) {
		error("failed to convert iat to binary\n");
		return -1;
	}

	uint64_t n_exp;
	if (hex_to_bin(&n_exp, sizeof(n_exp), &buffer[48], sizeof(n_exp) * 2) == -1) {
		error("failed to convert exp to binary\n");
		return -1;
	}

	bwt->iat = (time_t)ntohll(n_iat);
	bwt->exp = (time_t)ntohll(n_exp);

	return 0;
}

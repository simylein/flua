#include "bwt.h"
#include "config.h"
#include "endian.h"
#include "format.h"
#include "logger.h"
#include "request.h"
#include "sha256.h"
#include "string.h"
#include "time.h"
#include <stdint.h>

int sign_bwt(char (*buffer)[129], const uint8_t *id, const size_t id_len) {
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

	uint8_t hmac[32];
	sha256_hmac((uint8_t *)bwt_key, 6, buffer, id_len * 2 + sizeof(n_iat) * 2 + sizeof(n_exp) * 2, &hmac);
	if (bin_to_hex(*buffer + id_len * 2 + sizeof(n_iat) * 2 + sizeof(n_exp) * 2, sizeof(hmac) * 2 + 1, hmac, sizeof(hmac)) ==
			-1) {
		error("failed to convert hmac to hex\n");
		return -1;
	}

	return 0;
}

int verify_bwt(const char *cookie, bwt_t *bwt) {
	char buffer[129];
	if (sscanf(cookie, "auth=%128s\r\n", buffer) != 1) {
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
	if (hex_to_bin(&n_exp, sizeof(n_exp), &buffer[sizeof(bwt->id) * 2 + sizeof(n_iat) * 2], sizeof(n_exp) * 2) == -1) {
		error("failed to convert exp to binary\n");
		return -1;
	}

	uint8_t actual_hmac[32];
	if (hex_to_bin(actual_hmac, sizeof(actual_hmac), &buffer[sizeof(bwt->id) * 2 + sizeof(n_iat) * 2 + sizeof(n_exp) * 2],
								 sizeof(actual_hmac) * 2) == -1) {
		error("failed to convert hmac to binary\n");
	}

	uint8_t expected_hmac[32];
	sha256_hmac((uint8_t *)bwt_key, 6, buffer, sizeof(bwt->id) * 2 + sizeof(n_iat) * 2 + sizeof(n_exp) * 2, &expected_hmac);

	if (memcmp(actual_hmac, expected_hmac, sizeof(expected_hmac)) != 0) {
		warn("bwt %.8s has invalid signature\n", buffer);
		return -1;
	}

	bwt->iat = (time_t)ntohll(n_iat);
	bwt->exp = (time_t)ntohll(n_exp);

	time_t now = time(NULL);
	if (bwt->exp < now) {
		char time_buffer[8];
		human_time(&time_buffer, now - bwt->exp);
		warn("bwt %.8s has expired %s ago\n", buffer, time_buffer);
		return -1;
	}

	return 0;
}

#include "bwt.h"
#include "base64.h"
#include "config.h"
#include "endian.h"
#include "format.h"
#include "logger.h"
#include "request.h"
#include "sha256.h"
#include "string.h"
#include "time.h"
#include "utils.h"
#include <stdint.h>

int sign_bwt(char (*buffer)[88], const uint8_t *id, const size_t id_len) {
	const time_t iat = time(NULL);
	const uint64_t n_iat = htonll((uint64_t)iat);
	const time_t exp = iat + bwt_ttl;
	const uint64_t n_exp = htonll((uint64_t)exp);

	const size_t offset = id_len + sizeof(n_iat) + sizeof(n_exp);

	uint8_t binary[64];
	memcpy(binary, id, id_len);
	memcpy(binary + id_len, &n_iat, sizeof(n_iat));
	memcpy(binary + id_len + sizeof(n_iat), &n_exp, sizeof(n_exp));

	uint8_t hmac[32];
	sha256_hmac((uint8_t *)bwt_key, strlen(bwt_key), binary, offset, &hmac);
	memcpy(binary + offset, hmac, offset);

	if (base64_encode((char *)buffer, sizeof(*buffer), binary, sizeof(binary)) == -1) {
		error("failed to encode bwt in base 64\n");
		return -1;
	}

	return 0;
}

int verify_bwt(const char *cookie, const size_t cookie_len, bwt_t *bwt) {
	char *buffer;
	size_t buffer_len;
	if (strnfind(cookie, cookie_len, "auth=", "", &buffer, &buffer_len, 88) == -1) {
		warn("no auth value in cookie header\n");
		return -1;
	}

	uint8_t binary[64];
	if (base64_decode(binary, sizeof(binary), buffer, buffer_len) == -1) {
		error("failed to decode bwt from base 64\n");
		return -1;
	}

	uint64_t n_iat;
	uint64_t n_exp;
	memcpy(bwt->id, binary, sizeof(bwt->id));
	memcpy(&n_iat, binary + sizeof(bwt->id), sizeof(n_iat));
	memcpy(&n_exp, binary + sizeof(bwt->id) + sizeof(n_iat), sizeof(n_exp));

	const size_t offset = sizeof(bwt->id) + sizeof(n_iat) + sizeof(n_exp);

	uint8_t hmac[32];
	sha256_hmac((uint8_t *)bwt_key, strlen(bwt_key), binary, offset, &hmac);

	if (memcmp(binary + offset, hmac, sizeof(hmac)) != 0) {
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

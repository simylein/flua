#include "bwt.h"
#include "base32.h"
#include "config.h"
#include "endian.h"
#include "format.h"
#include "logger.h"
#include "sha256.h"
#include "string.h"
#include "strn.h"
#include <stdint.h>
#include <time.h>

int bwt_sign(char (*buffer)[109], uint8_t (*id)[16], uint8_t (*data)[4]) {
	const time_t iat = time(NULL);
	const time_t exp = iat + bwt_ttl;

	uint8_t binary[68];
	size_t offset = 0;
	memcpy(&binary[offset], id, sizeof(*id));
	offset += sizeof(*id);
	memcpy(&binary[offset], &(uint64_t[]){hton64((uint64_t)iat)}, sizeof(iat));
	offset += sizeof(iat);
	memcpy(&binary[offset], &(uint64_t[]){hton64((uint64_t)exp)}, sizeof(exp));
	offset += sizeof(exp);
	memcpy(&binary[offset], data, sizeof(*data));
	offset += sizeof(*data);

	uint8_t hmac[32];
	sha256_hmac((uint8_t *)bwt_key, strlen(bwt_key), binary, offset, &hmac);
	memcpy(&binary[offset], hmac, sizeof(hmac));

	if (base32_encode((char *)buffer, sizeof(*buffer), binary, sizeof(binary)) == -1) {
		error("failed to encode bwt in base 32\n");
		return -1;
	}

	return 0;
}

int bwt_verify(const char *cookie, const size_t cookie_len, bwt_t *bwt) {
	const char *buffer;
	size_t buffer_len;
	if (strnfind(cookie, cookie_len, "auth=", "", &buffer, &buffer_len, 109) == -1) {
		warn("no auth value in cookie header\n");
		return -1;
	}

	uint8_t binary[68];
	if (base32_decode(binary, sizeof(binary), buffer, buffer_len) == -1) {
		error("failed to decode bwt from base 32\n");
		return -1;
	}

	size_t offset = 0;
	memcpy(bwt->id, &binary[offset], sizeof(bwt->id));
	offset += sizeof(bwt->id);
	memcpy(&bwt->iat, &binary[offset], sizeof(bwt->iat));
	offset += sizeof(bwt->iat);
	memcpy(&bwt->exp, &binary[offset], sizeof(bwt->exp));
	offset += sizeof(bwt->exp);
	memcpy(bwt->data, &binary[offset], sizeof(bwt->data));
	offset += sizeof(bwt->data);

	uint8_t hmac[32];
	sha256_hmac((uint8_t *)bwt_key, strlen(bwt_key), binary, offset, &hmac);

	if (memcmp(binary + offset, hmac, sizeof(hmac)) != 0) {
		warn("bwt %.8s has invalid signature\n", buffer);
		return -1;
	}

	bwt->iat = (time_t)ntoh64((uint64_t)bwt->iat);
	bwt->exp = (time_t)ntoh64((uint64_t)bwt->exp);

	time_t now = time(NULL);
	if (bwt->exp < now) {
		char time_buffer[8];
		human_time(&time_buffer, now - bwt->exp);
		warn("bwt %.8s has expired %s ago\n", buffer, time_buffer);
		return -1;
	}

	return 0;
}

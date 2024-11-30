#include <memory.h>
#include <stdlib.h>

typedef struct sha256_ctx {
	uint32_t state[8];
	uint8_t data[64];
	size_t data_len;
	uint64_t bit_len;
} sha256_ctx;

const uint32_t constants[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
																0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
																0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
																0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
																0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
																0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
																0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
																0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

uint32_t rotate(uint32_t value, uint32_t bits) { return (((value) >> (bits)) | ((value) << (32 - (bits)))); }
uint32_t choose(uint32_t condition, uint32_t selector, uint32_t mask) {
	return (((condition) & (selector)) ^ (~(condition) & (mask)));
}
uint32_t majority(uint32_t candidate1, uint32_t candidate2, uint32_t candidate3) {
	return (((candidate1) & (candidate2)) ^ ((candidate1) & (candidate3)) ^ ((candidate2) & (candidate3)));
}
uint32_t epsilon0(uint32_t value) { return (rotate(value, 2) ^ rotate(value, 13) ^ rotate(value, 22)); }
uint32_t epsilon1(uint32_t value) { return (rotate(value, 6) ^ rotate(value, 11) ^ rotate(value, 25)); }
uint32_t sigma0(uint32_t value) { return (rotate(value, 7) ^ rotate(value, 18) ^ ((value) >> 3)); }
uint32_t sigma1(uint32_t value) { return (rotate(value, 17) ^ rotate(value, 19) ^ ((value) >> 10)); }

void sha256_init(sha256_ctx *ctx) {
	ctx->data_len = 0;
	ctx->bit_len = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}

void sha256_transform(sha256_ctx *ctx, const uint8_t *data) {
	uint32_t alpha, bravo, charlie, delta, echo, foxtrot, golf, hotel;
	uint32_t india, juliet;
	uint32_t store1, store2;
	uint32_t message[64];

	india = 0;
	juliet = 0;
	for (; india < 16; india++, juliet += 4)
		message[india] = ((uint32_t)(data[juliet] << 24)) | ((uint32_t)(data[juliet + 1]) << 16) |
										 ((uint32_t)(data[juliet + 2]) << 8) | ((uint32_t)(data[juliet + 3]));
	for (; india < 64; india++)
		message[india] = sigma1(message[india - 2]) + message[india - 7] + sigma0(message[india - 15]) + message[india - 16];

	alpha = ctx->state[0];
	bravo = ctx->state[1];
	charlie = ctx->state[2];
	delta = ctx->state[3];
	echo = ctx->state[4];
	foxtrot = ctx->state[5];
	golf = ctx->state[6];
	hotel = ctx->state[7];

	india = 0;
	for (; india < 64; india++) {
		store1 = hotel + epsilon1(echo) + choose(echo, foxtrot, golf) + constants[india] + message[india];
		store2 = epsilon0(alpha) + majority(alpha, bravo, charlie);
		hotel = golf;
		golf = foxtrot;
		foxtrot = echo;
		echo = delta + store1;
		delta = charlie;
		charlie = bravo;
		bravo = alpha;
		alpha = store1 + store2;
	}

	ctx->state[0] += alpha;
	ctx->state[1] += bravo;
	ctx->state[2] += charlie;
	ctx->state[3] += delta;
	ctx->state[4] += echo;
	ctx->state[5] += foxtrot;
	ctx->state[6] += golf;
	ctx->state[7] += hotel;
}

void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t data_len) {
	size_t index = 0;

	for (; index < data_len; index++) {
		ctx->data[ctx->data_len] = data[index];
		ctx->data_len++;
		if (ctx->data_len == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bit_len += 512;
			ctx->data_len = 0;
		}
	}
}

void sha256_final(sha256_ctx *ctx, uint8_t (*hash)[32]) {
	size_t index = ctx->data_len;

	if (ctx->data_len < 56) {
		ctx->data[index++] = 0x80;
		while (index < 56)
			ctx->data[index++] = 0x00;
	} else {
		ctx->data[index++] = 0x80;
		while (index < 64)
			ctx->data[index++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	ctx->bit_len += ctx->data_len * 8;
	ctx->data[63] = (uint8_t)ctx->bit_len;
	ctx->data[62] = (uint8_t)(ctx->bit_len >> 8);
	ctx->data[61] = (uint8_t)(ctx->bit_len >> 16);
	ctx->data[60] = (uint8_t)(ctx->bit_len >> 24);
	ctx->data[59] = (uint8_t)(ctx->bit_len >> 32);
	ctx->data[58] = (uint8_t)(ctx->bit_len >> 40);
	ctx->data[57] = (uint8_t)(ctx->bit_len >> 48);
	ctx->data[56] = (uint8_t)(ctx->bit_len >> 56);
	sha256_transform(ctx, ctx->data);

	for (index = 0; index < 4; index++) {
		(*hash)[index] = (ctx->state[0] >> (24 - index * 8)) & 0x000000ff;
		(*hash)[index + 4] = (ctx->state[1] >> (24 - index * 8)) & 0x000000ff;
		(*hash)[index + 8] = (ctx->state[2] >> (24 - index * 8)) & 0x000000ff;
		(*hash)[index + 12] = (ctx->state[3] >> (24 - index * 8)) & 0x000000ff;
		(*hash)[index + 16] = (ctx->state[4] >> (24 - index * 8)) & 0x000000ff;
		(*hash)[index + 20] = (ctx->state[5] >> (24 - index * 8)) & 0x000000ff;
		(*hash)[index + 24] = (ctx->state[6] >> (24 - index * 8)) & 0x000000ff;
		(*hash)[index + 28] = (ctx->state[7] >> (24 - index * 8)) & 0x000000ff;
	}
}

void sha256(const void *data, const size_t data_len, uint8_t (*hash)[32]) {
	struct sha256_ctx ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, data, data_len);
	sha256_final(&ctx, hash);
}

#include "../lib/logger.h"
#include "../lib/request.h"
#include "../lib/response.h"
#include "assemble.h"
#include "file.h"
#include "hydrate.h"
#include <pthread.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>

void serve(file_t *asset, uint8_t (*cull)(file_t *file), response_t *response) {
	if (file(asset, cull) == -1) {
		response->status = 500;
		return;
	}

	pthread_rwlock_rdlock(&asset->lock);
	if (asset->hydrated == false) {
		pthread_rwlock_unlock(&asset->lock);
		pthread_rwlock_wrlock(&asset->lock);
		if (asset->hydrated == true) {
			goto cleanup;
		}

		if (assemble(asset) == -1) {
			response->status = 500;
			goto cleanup;
		}

		class_t classes[256];
		uint16_t classes_len = 0;
		if (extract(asset, &classes, &classes_len) == -1) {
			response->status = 500;
			goto cleanup;
		}

		if (hydrate(asset, &classes, &classes_len) == -1) {
			response->status = 500;
			goto cleanup;
		}

		pthread_rwlock_unlock(&asset->lock);
		pthread_rwlock_rdlock(&asset->lock);
	}

	if (asset->ptr != NULL) {
		info("sending file %s\n", asset->path);

		if (asset->len > response->body.cap) {
			error("file length %zu exceeds buffer length %u\n", asset->len, response->body.cap);
			response->status = 500;
			goto cleanup;
		}

		if (response->status == 0) {
			response->status = 200;
		}
		header_write(response, "content-type:%s\r\n", type(asset->path));
		header_write(response, "content-length:%zu\r\n", asset->len);
		body_write(response, asset->ptr, asset->len);
	}

cleanup:
	pthread_rwlock_unlock(&asset->lock);
}

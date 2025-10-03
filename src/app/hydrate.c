#include "hydrate.h"
#include "../lib/error.h"
#include "../lib/logger.h"
#include "file.h"
#include "utility.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int append(class_t (*classes)[256], uint16_t *classes_len, char *next_ptr) {
	if ((size_t)*classes_len + 1 >= sizeof(*classes) / sizeof(class_t)) {
		error("can not handle more than %zu classes\n", sizeof(*classes) / sizeof(class_t));
		return -1;
	}

	if ((*classes)[*classes_len].len != 0) {
		bool unique = true;
		for (size_t index = 0; index < *classes_len; index++) {
			if ((*classes)[index].len == (*classes)[*classes_len].len &&
					memcmp((*classes)[index].ptr, (*classes)[*classes_len].ptr, (*classes)[index].len) == 0) {
				unique = false;
			}
		}
		if (unique) {
			(*classes_len)++;
		}
		(*classes)[*classes_len].len = 0;
	}

	if (next_ptr != NULL) {
		(*classes)[*classes_len].ptr = next_ptr;
	}

	return 0;
}

int extract(file_t *file, class_t (*classes)[256], uint16_t *classes_len) {
	bool tag = false;
	bool class = false;
	bool quote = false;

	(*classes)[*classes_len].len = 0;

	uint8_t stage = 0;
	size_t index = 0;

	while (stage == 0 && index < file->len) {
		char *byte = &file->ptr[index];
		if (index + 5 < file->len && *byte == '<' && memcmp(&file->ptr[index + 1], "body", 4) == 0) {
			tag = true;
			stage = 1;
		}
		index++;
	}

	while (stage == 1 && index < file->len) {
		char *byte = &file->ptr[index];
		if (*byte == '<' && !tag && !quote) {
			if (index + 7 < file->len && *byte == '<' && memcmp(&file->ptr[index + 1], "script", 6) == 0) {
				stage = 2;
			}
			tag = true;
		} else if (*byte == '>' && tag && !quote) {
			tag = false;
		} else if (*byte == '"' && tag && !quote) {
			quote = true;
			if (index > 6 && memcmp(&file->ptr[index - 6], "class=", 6) == 0) {
				if (append(classes, classes_len, &file->ptr[index + 1]) == -1) {
					return -1;
				}
				class = true;
			}
		} else if (*byte == '"' && tag && quote) {
			quote = false;
			if (class) {
				if (append(classes, classes_len, NULL) == -1) {
					return -1;
				}
				class = false;
			}
		} else if (tag && class && quote) {
			if (*byte == ' ') {
				if (append(classes, classes_len, &file->ptr[index + 1]) == -1) {
					return -1;
				}
			} else {
				(*classes)[*classes_len].len++;
			}
		}
		index++;
	}

	while (stage == 2 && index < file->len) {
		char *byte = &file->ptr[index];
		if (*byte == '(' && !tag && !class) {
			if (index > 13 && memcmp(&file->ptr[index - 13], "classList.add", 13) == 0) {
				tag = true;
			}
		} else if (*byte == ')' && tag && !quote) {
			tag = false;
		} else if (*byte == '\'' && tag && !quote) {
			quote = true;
			if (append(classes, classes_len, &file->ptr[index + 1]) == -1) {
				return -1;
			}
		} else if (*byte == '\'' && tag && quote) {
			quote = false;
			if (append(classes, classes_len, NULL) == -1) {
				return -1;
			}
		} else if (tag && quote) {
			if (*byte == ',') {
				if (append(classes, classes_len, &file->ptr[index + 1]) == -1) {
					return -1;
				}
			} else {
				(*classes)[*classes_len].len++;
			}
		}
		index++;
	}

	return 0;
}

int hydrate(file_t *file, class_t (*classes)[256], uint16_t *classes_len) {
	trace("extracted %hu classes\n", *classes_len);

	char global[8192];
	uint16_t global_len = 0;
	breakpoint_t global_breakpoint = {
			.tag = "",
			.tag_len = 0,
	};

	char tiny[8192];
	uint16_t tiny_len = 0;
	breakpoint_t tiny_breakpoint = {
			.tag = "xs",
			.tag_len = 2,
			.prefix = "@media (min-width:512px){",
			.prefix_len = 25,
			.suffix = "}",
			.suffix_len = 1,
	};

	char small[8192];
	uint16_t small_len = 0;
	breakpoint_t small_breakpoint = {
			.tag = "sm",
			.tag_len = 2,
			.prefix = "@media (min-width:640px){",
			.prefix_len = 25,
			.suffix = "}",
			.suffix_len = 1,
	};

	char medium[8192];
	uint16_t medium_len = 0;
	breakpoint_t medium_breakpoint = {
			.tag = "md",
			.tag_len = 2,
			.prefix = "@media (min-width:768px){",
			.prefix_len = 25,
			.suffix = "}",
			.suffix_len = 1,
	};

	char large[8192];
	uint16_t large_len = 0;
	breakpoint_t large_breakpoint = {
			.tag = "lg",
			.tag_len = 2,
			.prefix = "@media (min-width:1024px){",
			.prefix_len = 26,
			.suffix = "}",
			.suffix_len = 1,
	};

	char giant[8192];
	uint16_t giant_len = 0;
	breakpoint_t giant_breakpoint = {
			.tag = "xl",
			.tag_len = 2,
			.prefix = "@media (min-width:1280px){",
			.prefix_len = 26,
			.suffix = "}",
			.suffix_len = 1,
	};

	char dark[8192];
	uint16_t dark_len = 0;
	breakpoint_t dark_breakpoint = {
			.tag = "dark",
			.tag_len = 4,
			.prefix = "@media (prefers-color-scheme:dark){",
			.prefix_len = 35,
			.suffix = "}",
			.suffix_len = 1,
	};

	debug("hydrating file %s\n", file->path);
	for (uint16_t index = 0; index < *classes_len; index++) {
		class_t pfx;
		class_t cls;

		uint8_t stg = 0;
		uint16_t ind = 0;

		if (memchr((*classes)[index].ptr, ':', (*classes)[index].len) == NULL) {
			stg = 1;
		}

		pfx.len = 0;
		pfx.ptr = &(*classes)[index].ptr[ind];
		while (stg == 0 && ind < (*classes)[index].len) {
			char *byte = &(*classes)[index].ptr[ind];
			if (*byte == ':') {
				stg = 1;
			} else {
				pfx.len++;
			}
			ind++;
		}

		cls.len = 0;
		cls.ptr = &(*classes)[index].ptr[ind];
		cls.known = false;
		while (stg == 1 && ind < (*classes)[index].len) {
			cls.len++;
			ind++;
		}

		common(&cls, &global, &global_len);
		position(&cls, &global, &global_len);
		display(&pfx, &cls, &global, &global_len, &global_breakpoint);
		display(&pfx, &cls, &tiny, &tiny_len, &tiny_breakpoint);
		display(&pfx, &cls, &small, &small_len, &small_breakpoint);
		display(&pfx, &cls, &medium, &medium_len, &medium_breakpoint);
		display(&pfx, &cls, &large, &large_len, &large_breakpoint);
		display(&pfx, &cls, &giant, &giant_len, &giant_breakpoint);
		spacing(&pfx, &cls, &global, &global_len, &global_breakpoint);
		spacing(&pfx, &cls, &tiny, &tiny_len, &tiny_breakpoint);
		spacing(&pfx, &cls, &small, &small_len, &small_breakpoint);
		spacing(&pfx, &cls, &medium, &medium_len, &medium_breakpoint);
		spacing(&pfx, &cls, &large, &large_len, &large_breakpoint);
		spacing(&pfx, &cls, &giant, &giant_len, &giant_breakpoint);
		sizing(&pfx, &cls, &global, &global_len, &global_breakpoint);
		sizing(&pfx, &cls, &tiny, &tiny_len, &tiny_breakpoint);
		sizing(&pfx, &cls, &small, &small_len, &small_breakpoint);
		sizing(&pfx, &cls, &medium, &medium_len, &medium_breakpoint);
		sizing(&pfx, &cls, &large, &large_len, &large_breakpoint);
		sizing(&pfx, &cls, &giant, &giant_len, &giant_breakpoint);
		border(&pfx, &cls, &global, &global_len, &global_breakpoint);
		border(&pfx, &cls, &tiny, &tiny_len, &tiny_breakpoint);
		border(&pfx, &cls, &small, &small_len, &small_breakpoint);
		border(&pfx, &cls, &medium, &medium_len, &medium_breakpoint);
		border(&pfx, &cls, &large, &large_len, &large_breakpoint);
		border(&pfx, &cls, &giant, &giant_len, &giant_breakpoint);
		overflow(&pfx, &cls, &global, &global_len, &global_breakpoint);
		overflow(&pfx, &cls, &tiny, &tiny_len, &tiny_breakpoint);
		overflow(&pfx, &cls, &small, &small_len, &small_breakpoint);
		overflow(&pfx, &cls, &medium, &medium_len, &medium_breakpoint);
		overflow(&pfx, &cls, &large, &large_len, &large_breakpoint);
		overflow(&pfx, &cls, &giant, &giant_len, &giant_breakpoint);
		flex(&pfx, &cls, &global, &global_len, &global_breakpoint);
		flex(&pfx, &cls, &tiny, &tiny_len, &tiny_breakpoint);
		flex(&pfx, &cls, &small, &small_len, &small_breakpoint);
		flex(&pfx, &cls, &medium, &medium_len, &medium_breakpoint);
		flex(&pfx, &cls, &large, &large_len, &large_breakpoint);
		flex(&pfx, &cls, &giant, &giant_len, &giant_breakpoint);
		text(&cls, &global, &global_len);
		font(&cls, &global, &global_len);
		color(&pfx, &cls, &global, &global_len, &global_breakpoint);
		color(&pfx, &cls, &dark, &dark_len, &dark_breakpoint);
		opacity(&cls, &global, &global_len);
		cursor(&cls, &global, &global_len);
		layout(&cls, &global, &global_len);
		table(&cls, &global, &global_len);

		if (cls.known == false) {
			warn("unknown class %.*s\n", (*classes)[index].len, (*classes)[index].ptr);
		}
	}

	size_t len = file->len + global_len + tiny_len + small_len + medium_len + large_len + giant_len + dark_len;
	if (tiny_len > 0) {
		len += tiny_breakpoint.prefix_len;
		len += tiny_breakpoint.suffix_len;
	}
	if (small_len > 0) {
		len += small_breakpoint.prefix_len;
		len += small_breakpoint.suffix_len;
	}
	if (medium_len > 0) {
		len += medium_breakpoint.prefix_len;
		len += medium_breakpoint.suffix_len;
	}
	if (large_len > 0) {
		len += large_breakpoint.prefix_len;
		len += large_breakpoint.suffix_len;
	}
	if (giant_len > 0) {
		len += giant_breakpoint.prefix_len;
		len += giant_breakpoint.suffix_len;
	}
	if (dark_len > 0) {
		len += dark_breakpoint.prefix_len;
		len += dark_breakpoint.suffix_len;
	}
	char *ptr = malloc(len);
	if (ptr == NULL) {
		error("failed to allocate %zu bytes for %s because %s\n", len, file->path, errno_str());
		return -1;
	}

	bool tag = false;
	size_t ptr_ind = 0;
	size_t file_ind = 0;
	while (file_ind < file->len) {
		ptr[ptr_ind] = file->ptr[file_ind];
		if (file_ind > 13 && memcmp(&file->ptr[file_ind - 7], "<style>", 7) == 0) {
			tag = true;
			break;
		}
		ptr_ind++;
		file_ind++;
	}

	if (*classes_len != 0 && tag == false) {
		error("file %s does not contain a style tag\n", file->path);
		return -1;
	}

	if (global_len != 0) {
		memcpy(&ptr[ptr_ind], global, global_len);
		ptr_ind += global_len;
	}

	if (tiny_len != 0) {
		memcpy(&ptr[ptr_ind], tiny_breakpoint.prefix, tiny_breakpoint.prefix_len);
		ptr_ind += tiny_breakpoint.prefix_len;
		memcpy(&ptr[ptr_ind], small, tiny_len);
		ptr_ind += tiny_len;
		memcpy(&ptr[ptr_ind], tiny_breakpoint.suffix, tiny_breakpoint.suffix_len);
		ptr_ind += tiny_breakpoint.suffix_len;
	}

	if (small_len != 0) {
		memcpy(&ptr[ptr_ind], small_breakpoint.prefix, small_breakpoint.prefix_len);
		ptr_ind += small_breakpoint.prefix_len;
		memcpy(&ptr[ptr_ind], small, small_len);
		ptr_ind += small_len;
		memcpy(&ptr[ptr_ind], small_breakpoint.suffix, small_breakpoint.suffix_len);
		ptr_ind += small_breakpoint.suffix_len;
	}

	if (medium_len != 0) {
		memcpy(&ptr[ptr_ind], medium_breakpoint.prefix, medium_breakpoint.prefix_len);
		ptr_ind += medium_breakpoint.prefix_len;
		memcpy(&ptr[ptr_ind], medium, medium_len);
		ptr_ind += medium_len;
		memcpy(&ptr[ptr_ind], medium_breakpoint.suffix, medium_breakpoint.suffix_len);
		ptr_ind += medium_breakpoint.suffix_len;
	}

	if (large_len != 0) {
		memcpy(&ptr[ptr_ind], large_breakpoint.prefix, large_breakpoint.prefix_len);
		ptr_ind += large_breakpoint.prefix_len;
		memcpy(&ptr[ptr_ind], large, large_len);
		ptr_ind += large_len;
		memcpy(&ptr[ptr_ind], large_breakpoint.suffix, large_breakpoint.suffix_len);
		ptr_ind += large_breakpoint.suffix_len;
	}

	if (giant_len != 0) {
		memcpy(&ptr[ptr_ind], giant_breakpoint.prefix, giant_breakpoint.prefix_len);
		ptr_ind += giant_breakpoint.prefix_len;
		memcpy(&ptr[ptr_ind], giant, giant_len);
		ptr_ind += giant_len;
		memcpy(&ptr[ptr_ind], giant_breakpoint.suffix, giant_breakpoint.suffix_len);
		ptr_ind += giant_breakpoint.suffix_len;
	}

	if (dark_len != 0) {
		memcpy(&ptr[ptr_ind], dark_breakpoint.prefix, dark_breakpoint.prefix_len);
		ptr_ind += dark_breakpoint.prefix_len;
		memcpy(&ptr[ptr_ind], dark, dark_len);
		ptr_ind += dark_len;
		memcpy(&ptr[ptr_ind], dark_breakpoint.suffix, dark_breakpoint.suffix_len);
		ptr_ind += dark_breakpoint.suffix_len;
	}

	memcpy(&ptr[ptr_ind], &file->ptr[file_ind], file->len - file_ind);
	ptr_ind += file->len - file_ind;
	file_ind += file->len - file_ind;

	free(file->ptr);
	file->ptr = ptr;
	file->len = len;
	file->hydrated = true;

	return 0;
}

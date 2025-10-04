#include "utility.h"
#include "hydrate.h"
#include <stdbool.h>
#include <string.h>

bool stamp(class_t *pfx, class_t *cls, const keymap_t *variant, const keymap_t *mapping, const keymap_t *suffix,
					 char (*buffer)[8192], uint16_t *buffer_len) {
	if (*buffer_len < sizeof(*buffer) - 64) {
		(*buffer)[*buffer_len] = '.';
		*buffer_len += 1;

		if (pfx != NULL) {
			if (pfx->len > 0) {
				memcpy(&(*buffer)[*buffer_len], pfx->ptr, pfx->len);
				*buffer_len += pfx->len;

				memcpy(&(*buffer)[*buffer_len], "\\:", 2);
				*buffer_len += 2;
			}
		}

		for (uint8_t ind = 0; ind < cls->len; ind++) {
			if (cls->ptr[ind] == '.' || cls->ptr[ind] == '/') {
				(*buffer)[*buffer_len] = '\\';
				*buffer_len += 1;
			}
			(*buffer)[*buffer_len] = cls->ptr[ind];
			*buffer_len += 1;
		}

		(*buffer)[*buffer_len] = '{';
		*buffer_len += 1;

		memcpy(&(*buffer)[*buffer_len], variant->val, variant->val_len);
		*buffer_len += variant->val_len;

		if (mapping != NULL) {
			(*buffer)[*buffer_len] = ':';
			*buffer_len += 1;

			memcpy(&(*buffer)[*buffer_len], mapping->val, mapping->val_len);
			*buffer_len += mapping->val_len;
		}

		if (suffix != NULL) {
			memcpy(&(*buffer)[*buffer_len], suffix->val, suffix->val_len);
			*buffer_len += suffix->val_len;
		}

		(*buffer)[*buffer_len] = '}';
		*buffer_len += 1;

		return true;
	}
	return false;
}

void common(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t commons[] = {
			{.key = "font-sans",
			 .key_len = 9,
			 .val = "font-family:ui-sans-serif,system-ui,sans-serif,"
							"'Apple Color Emoji','Segoe UI Emoji','Segoe UI Symbol','Noto Color Emoji'",
			 .val_len = 120},
			{.key = "font-serif",
			 .key_len = 10,
			 .val = "font-family:ui-serif,"
							"Georgia,Cambria,'Times New Roman',Times,serif",
			 .val_len = 66},
			{.key = "font-mono",
			 .key_len = 9,
			 .val = "font-family:ui-monospace,"
							"SFMono-Regular,Menlo,Monaco,Consolas,'Liberation Mono','Courier New',monospace",
			 .val_len = 103},
			{.key = "box-border", .key_len = 10, .val = "box-sizing:border-box", .val_len = 21},
			{.key = "box-content", .key_len = 11, .val = "box-sizing:content-box", .val_len = 22},
	};

	for (uint8_t index = 0; index < sizeof(commons) / sizeof(keymap_t); index++) {
		if (cls->len == commons[index].key_len && memcmp(cls->ptr, commons[index].key, commons[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &commons[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void position(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t positions[] = {
			{.key = "static", .key_len = 6, .val = "position:static", .val_len = 15},
			{.key = "relative", .key_len = 8, .val = "position:relative", .val_len = 17},
			{.key = "absolute", .key_len = 8, .val = "position:absolute", .val_len = 17},
			{.key = "fixed", .key_len = 5, .val = "position:fixed", .val_len = 14},
			{.key = "sticky", .key_len = 6, .val = "position:sticky", .val_len = 15},
	};

	for (uint8_t index = 0; index < sizeof(positions) / sizeof(keymap_t); index++) {
		if (cls->len == positions[index].key_len && memcmp(cls->ptr, positions[index].key, positions[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &positions[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void display(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint) {
	if (pfx->len != breakpoint->tag_len || memcmp(pfx->ptr, breakpoint->tag, breakpoint->tag_len) != 0) {
		return;
	}

	const keymap_t displays[] = {
			{.key = "block", .key_len = 5, .val = "display:block", .val_len = 13},
			{.key = "inline", .key_len = 6, .val = "display:inline", .val_len = 14},
			{.key = "flex", .key_len = 4, .val = "display:flex", .val_len = 12},
			{.key = "grid", .key_len = 4, .val = "display:grid", .val_len = 12},
			{.key = "hidden", .key_len = 6, .val = "display:none", .val_len = 12},
			{.key = "contents", .key_len = 8, .val = "display:contents", .val_len = 16},
	};

	for (uint8_t index = 0; index < sizeof(displays) / sizeof(keymap_t); index++) {
		if (cls->len == displays[index].key_len && memcmp(cls->ptr, displays[index].key, displays[index].key_len) == 0) {
			cls->known = stamp(pfx, cls, &displays[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void spacing(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint) {
	if (pfx->len != breakpoint->tag_len || memcmp(pfx->ptr, breakpoint->tag, breakpoint->tag_len) != 0) {
		return;
	}

	const keymap_t variants[] = {
			{.key = "m-", .key_len = 2, .val = "margin", .val_len = 6},
			{.key = "mt-", .key_len = 3, .val = "margin-top", .val_len = 10},
			{.key = "mr-", .key_len = 3, .val = "margin-right", .val_len = 12},
			{.key = "mb-", .key_len = 3, .val = "margin-bottom", .val_len = 13},
			{.key = "ml-", .key_len = 3, .val = "margin-left", .val_len = 11},
			{.key = "p-", .key_len = 2, .val = "padding", .val_len = 7},
			{.key = "pt-", .key_len = 3, .val = "padding-top", .val_len = 11},
			{.key = "pr-", .key_len = 3, .val = "padding-right", .val_len = 13},
			{.key = "pb-", .key_len = 3, .val = "padding-bottom", .val_len = 14},
			{.key = "pl-", .key_len = 3, .val = "padding-left", .val_len = 12},
			{.key = "top-", .key_len = 4, .val = "top", .val_len = 3},
			{.key = "right-", .key_len = 6, .val = "right", .val_len = 5},
			{.key = "bottom-", .key_len = 7, .val = "bottom", .val_len = 6},
			{.key = "left-", .key_len = 5, .val = "left", .val_len = 4},
	};

	const keymap_t mappings[] = {
			{.key = "0", .key_len = 1, .val = "0px", .val_len = 3},		 {.key = "0.5", .key_len = 3, .val = "2px", .val_len = 3},
			{.key = "1", .key_len = 1, .val = "4px", .val_len = 3},		 {.key = "1.5", .key_len = 3, .val = "6px", .val_len = 3},
			{.key = "2", .key_len = 1, .val = "8px", .val_len = 3},		 {.key = "2.5", .key_len = 3, .val = "10px", .val_len = 4},
			{.key = "3", .key_len = 1, .val = "12px", .val_len = 4},	 {.key = "3.5", .key_len = 3, .val = "14px", .val_len = 4},
			{.key = "4", .key_len = 1, .val = "16px", .val_len = 4},	 {.key = "4.5", .key_len = 3, .val = "18px", .val_len = 4},
			{.key = "5", .key_len = 1, .val = "20px", .val_len = 4},	 {.key = "5.5", .key_len = 3, .val = "22px", .val_len = 4},
			{.key = "6", .key_len = 1, .val = "24px", .val_len = 4},	 {.key = "6.5", .key_len = 3, .val = "26px", .val_len = 4},
			{.key = "7", .key_len = 1, .val = "28px", .val_len = 4},	 {.key = "7.5", .key_len = 3, .val = "30px", .val_len = 4},
			{.key = "8", .key_len = 1, .val = "32px", .val_len = 4},	 {.key = "9", .key_len = 1, .val = "36px", .val_len = 4},
			{.key = "10", .key_len = 2, .val = "40px", .val_len = 4},	 {.key = "11", .key_len = 2, .val = "44px", .val_len = 4},
			{.key = "12", .key_len = 2, .val = "48px", .val_len = 4},	 {.key = "13", .key_len = 2, .val = "52px", .val_len = 4},
			{.key = "14", .key_len = 2, .val = "56px", .val_len = 4},	 {.key = "15", .key_len = 2, .val = "60px", .val_len = 4},
			{.key = "16", .key_len = 2, .val = "64px", .val_len = 4},	 {.key = "20", .key_len = 2, .val = "80px", .val_len = 4},
			{.key = "24", .key_len = 2, .val = "96px", .val_len = 4},	 {.key = "28", .key_len = 2, .val = "112px", .val_len = 5},
			{.key = "32", .key_len = 2, .val = "128px", .val_len = 5}, {.key = "40", .key_len = 2, .val = "160px", .val_len = 5},
			{.key = "48", .key_len = 2, .val = "192px", .val_len = 5}, {.key = "56", .key_len = 2, .val = "224px", .val_len = 5},
			{.key = "64", .key_len = 2, .val = "256px", .val_len = 5}, {.key = "72", .key_len = 2, .val = "288px", .val_len = 5},
			{.key = "80", .key_len = 2, .val = "320px", .val_len = 5}, {.key = "88", .key_len = 2, .val = "352px", .val_len = 5},
			{.key = "96", .key_len = 2, .val = "384px", .val_len = 5},
	};

	for (uint8_t index = 0; index < sizeof(variants) / sizeof(keymap_t); index++) {
		if (cls->len > variants[index].key_len && memcmp(cls->ptr, variants[index].key, variants[index].key_len) == 0) {
			const keymap_t *mapping = NULL;
			for (uint8_t ind = 0; ind < sizeof(mappings) / sizeof(keymap_t); ind++) {
				if (cls->len - variants[index].key_len == mappings[ind].key_len &&
						memcmp(&cls->ptr[variants[index].key_len], mappings[ind].key, mappings[ind].key_len) == 0) {
					mapping = &mappings[ind];
					break;
				}
			}
			if (mapping == NULL) {
				return;
			}

			cls->known = stamp(pfx, cls, &variants[index], mapping, NULL, buffer, buffer_len);
		}
	}
}

void sizing(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint) {
	if (pfx->len != breakpoint->tag_len || memcmp(pfx->ptr, breakpoint->tag, breakpoint->tag_len) != 0) {
		return;
	}

	const keymap_t variants[] = {
			{.key = "w-", .key_len = 2, .val = "width", .val_len = 5},
			{.key = "min-w-", .key_len = 6, .val = "min-width", .val_len = 9},
			{.key = "max-w-", .key_len = 6, .val = "max-width", .val_len = 9},
			{.key = "h-", .key_len = 2, .val = "height", .val_len = 6},
			{.key = "min-h-", .key_len = 6, .val = "min-height", .val_len = 10},
			{.key = "max-h-", .key_len = 6, .val = "max-height", .val_len = 10},
			{.key = "gap-", .key_len = 4, .val = "gap", .val_len = 3},
	};

	const keymap_t mappings[] = {
			{.key = "0", .key_len = 1, .val = "0px", .val_len = 3},
			{.key = "0.5", .key_len = 3, .val = "2px", .val_len = 3},
			{.key = "1", .key_len = 1, .val = "4px", .val_len = 3},
			{.key = "1.5", .key_len = 3, .val = "6px", .val_len = 3},
			{.key = "2", .key_len = 1, .val = "8px", .val_len = 3},
			{.key = "2.5", .key_len = 3, .val = "10px", .val_len = 4},
			{.key = "3", .key_len = 1, .val = "12px", .val_len = 4},
			{.key = "3.5", .key_len = 3, .val = "14px", .val_len = 4},
			{.key = "4", .key_len = 1, .val = "16px", .val_len = 4},
			{.key = "4.5", .key_len = 3, .val = "18px", .val_len = 4},
			{.key = "5", .key_len = 1, .val = "20px", .val_len = 4},
			{.key = "5.5", .key_len = 3, .val = "22px", .val_len = 4},
			{.key = "6", .key_len = 1, .val = "24px", .val_len = 4},
			{.key = "6.5", .key_len = 3, .val = "26px", .val_len = 4},
			{.key = "7", .key_len = 1, .val = "28px", .val_len = 4},
			{.key = "7.5", .key_len = 3, .val = "30px", .val_len = 4},
			{.key = "8", .key_len = 1, .val = "32px", .val_len = 4},
			{.key = "9", .key_len = 1, .val = "36px", .val_len = 4},
			{.key = "10", .key_len = 2, .val = "40px", .val_len = 4},
			{.key = "11", .key_len = 2, .val = "44px", .val_len = 4},
			{.key = "12", .key_len = 2, .val = "48px", .val_len = 4},
			{.key = "13", .key_len = 2, .val = "52px", .val_len = 4},
			{.key = "14", .key_len = 2, .val = "56px", .val_len = 4},
			{.key = "15", .key_len = 2, .val = "60px", .val_len = 4},
			{.key = "16", .key_len = 2, .val = "64px", .val_len = 4},
			{.key = "20", .key_len = 2, .val = "80px", .val_len = 4},
			{.key = "24", .key_len = 2, .val = "96px", .val_len = 4},
			{.key = "28", .key_len = 2, .val = "112px", .val_len = 5},
			{.key = "32", .key_len = 2, .val = "128px", .val_len = 5},
			{.key = "40", .key_len = 2, .val = "160px", .val_len = 5},
			{.key = "48", .key_len = 2, .val = "192px", .val_len = 5},
			{.key = "56", .key_len = 2, .val = "224px", .val_len = 5},
			{.key = "64", .key_len = 2, .val = "256px", .val_len = 5},
			{.key = "72", .key_len = 2, .val = "288px", .val_len = 5},
			{.key = "80", .key_len = 2, .val = "320px", .val_len = 5},
			{.key = "88", .key_len = 2, .val = "352px", .val_len = 5},
			{.key = "96", .key_len = 2, .val = "384px", .val_len = 5},
			{.key = "xs", .key_len = 2, .val = "512px", .val_len = 5},
			{.key = "sm", .key_len = 2, .val = "640px", .val_len = 5},
			{.key = "md", .key_len = 2, .val = "768px", .val_len = 5},
			{.key = "lg", .key_len = 2, .val = "1024px", .val_len = 6},
			{.key = "xl", .key_len = 2, .val = "1280px", .val_len = 6},
			{.key = "2xl", .key_len = 3, .val = "1536px", .val_len = 6},
			{.key = "full", .key_len = 4, .val = "100%", .val_len = 4},
			{.key = "vh", .key_len = 2, .val = "100vh", .val_len = 5},
			{.key = "svh", .key_len = 3, .val = "100svh", .val_len = 6},
			{.key = "lvh", .key_len = 3, .val = "100lvh", .val_len = 6},
			{.key = "dvh", .key_len = 3, .val = "100dvh", .val_len = 6},
			{.key = "vw", .key_len = 2, .val = "100vw", .val_len = 5},
			{.key = "svw", .key_len = 3, .val = "100svw", .val_len = 6},
			{.key = "lvw", .key_len = 3, .val = "100lvw", .val_len = 6},
			{.key = "dvw", .key_len = 3, .val = "100dvw", .val_len = 6},
			{.key = "min", .key_len = 3, .val = "min-content", .val_len = 11},
			{.key = "max", .key_len = 3, .val = "max-content", .val_len = 11},
			{.key = "fit", .key_len = 3, .val = "fit-content", .val_len = 11},
	};

	for (uint8_t index = 0; index < sizeof(variants) / sizeof(keymap_t); index++) {
		if (cls->len > variants[index].key_len && memcmp(cls->ptr, variants[index].key, variants[index].key_len) == 0) {
			const keymap_t *mapping = NULL;
			for (uint8_t ind = 0; ind < sizeof(mappings) / sizeof(keymap_t); ind++) {
				if (cls->len - variants[index].key_len == mappings[ind].key_len &&
						memcmp(&cls->ptr[variants[index].key_len], mappings[ind].key, mappings[ind].key_len) == 0) {
					mapping = &mappings[ind];
					break;
				}
			}
			if (mapping == NULL) {
				return;
			}

			cls->known = stamp(pfx, cls, &variants[index], mapping, NULL, buffer, buffer_len);
		}
	}
}

void border(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint) {
	if (pfx->len != breakpoint->tag_len || memcmp(pfx->ptr, breakpoint->tag, breakpoint->tag_len) != 0) {
		return;
	}

	const keymap_t borders[] = {
			{.key = "border-solid", .key_len = 12, .val = "border-style:solid", .val_len = 18},
			{.key = "border-dashed", .key_len = 13, .val = "border-style:dashed", .val_len = 19},
			{.key = "border-dotted", .key_len = 13, .val = "border-style:dotted", .val_len = 19},
			{.key = "border-double", .key_len = 13, .val = "border-style:double", .val_len = 20},
			{.key = "border-hidden", .key_len = 13, .val = "border-style:hidden", .val_len = 19},
			{.key = "border-none", .key_len = 11, .val = "border-style:none", .val_len = 17},
	};

	for (uint8_t index = 0; index < sizeof(borders) / sizeof(keymap_t); index++) {
		if (cls->len == borders[index].key_len && memcmp(cls->ptr, borders[index].key, borders[index].key_len) == 0) {
			cls->known = stamp(pfx, cls, &borders[index], NULL, NULL, buffer, buffer_len);
		}
	}

	const keymap_t variants[] = {
			{.key = "border-t-", .key_len = 9, .val = "border-top-width", .val_len = 16},
			{.key = "border-r-", .key_len = 9, .val = "border-right-width", .val_len = 18},
			{.key = "border-b-", .key_len = 9, .val = "border-bottom-width", .val_len = 19},
			{.key = "border-l-", .key_len = 9, .val = "border-left-width", .val_len = 17},
			{.key = "border-spacing-", .key_len = 15, .val = "border-spacing", .val_len = 14},
			{.key = "border-", .key_len = 7, .val = "border-width", .val_len = 12},
	};

	const keymap_t mappings[] = {
			{.key = "0", .key_len = 1, .val = "0px", .val_len = 3}, {.key = "1", .key_len = 1, .val = "1px", .val_len = 3},
			{.key = "2", .key_len = 1, .val = "2px", .val_len = 3}, {.key = "3", .key_len = 1, .val = "3px", .val_len = 3},
			{.key = "4", .key_len = 1, .val = "4px", .val_len = 3}, {.key = "6", .key_len = 1, .val = "6px", .val_len = 3},
			{.key = "8", .key_len = 1, .val = "8px", .val_len = 3},
	};

	for (uint8_t index = 0; index < sizeof(variants) / sizeof(keymap_t); index++) {
		if (cls->len > variants[index].key_len && memcmp(cls->ptr, variants[index].key, variants[index].key_len) == 0) {
			const keymap_t *mapping = NULL;
			for (uint8_t ind = 0; ind < sizeof(mappings) / sizeof(keymap_t); ind++) {
				if (cls->len - variants[index].key_len == mappings[ind].key_len &&
						memcmp(&cls->ptr[variants[index].key_len], mappings[ind].key, mappings[ind].key_len) == 0) {
					mapping = &mappings[ind];
					break;
				}
			}
			if (mapping == NULL) {
				return;
			}

			cls->known = stamp(pfx, cls, &variants[index], mapping, NULL, buffer, buffer_len);
		}
	}

	const keymap_t outlines[] = {
			{.key = "outline-0", .key_len = 9, .val = "outline-width:0px", .val_len = 17},
			{.key = "outline-1", .key_len = 9, .val = "outline-width:1px", .val_len = 17},
			{.key = "outline-2", .key_len = 9, .val = "outline-width:2px", .val_len = 17},
			{.key = "outline-4", .key_len = 9, .val = "outline-width:4px", .val_len = 17},
			{.key = "outline-8", .key_len = 9, .val = "outline-width:8px", .val_len = 17},
			{.key = "outline-none", .key_len = 12, .val = "outline-style:none", .val_len = 18},
			{.key = "outline-solid", .key_len = 13, .val = "outline-style:solid", .val_len = 19},
			{.key = "outline-dashed", .key_len = 14, .val = "outline-style:dashed", .val_len = 20},
			{.key = "outline-dotted", .key_len = 14, .val = "outline-style:dotted", .val_len = 20},
			{.key = "outline-double", .key_len = 14, .val = "outline-style:double", .val_len = 20},
	};

	for (uint8_t index = 0; index < sizeof(outlines) / sizeof(keymap_t); index++) {
		if (cls->len == outlines[index].key_len && memcmp(cls->ptr, outlines[index].key, outlines[index].key_len) == 0) {
			cls->known = stamp(pfx, cls, &outlines[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void overflow(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint) {
	if (pfx->len != breakpoint->tag_len || memcmp(pfx->ptr, breakpoint->tag, breakpoint->tag_len) != 0) {
		return;
	}

	const keymap_t overflows[] = {
			{.key = "scrollbar-auto", .key_len = 14, .val = "scrollbar-width:auto", .val_len = 20},
			{.key = "scrollbar-thin", .key_len = 14, .val = "scrollbar-width:thin", .val_len = 20},
			{.key = "scrollbar-none", .key_len = 14, .val = "scrollbar-width:none", .val_len = 20},
	};

	for (uint8_t index = 0; index < sizeof(overflows) / sizeof(keymap_t); index++) {
		if (cls->len == overflows[index].key_len && memcmp(cls->ptr, overflows[index].key, overflows[index].key_len) == 0) {
			cls->known = stamp(pfx, cls, &overflows[index], NULL, NULL, buffer, buffer_len);
		}
	}

	const keymap_t variants[] = {
			{.key = "overflow-x-", .key_len = 11, .val = "overflow-x", .val_len = 10},
			{.key = "overflow-y-", .key_len = 11, .val = "overflow-y", .val_len = 10},
			{.key = "overflow-", .key_len = 9, .val = "overflow", .val_len = 8},
	};

	const keymap_t mappings[] = {
			{.key = "auto", .key_len = 4, .val = "auto", .val_len = 4},
			{.key = "hidden", .key_len = 6, .val = "hidden", .val_len = 6},
			{.key = "clip", .key_len = 4, .val = "clip", .val_len = 4},
			{.key = "visible", .key_len = 7, .val = "visible", .val_len = 7},
			{.key = "scroll", .key_len = 6, .val = "scroll", .val_len = 6},
	};

	for (uint8_t index = 0; index < sizeof(variants) / sizeof(keymap_t); index++) {
		if (cls->len > variants[index].key_len && memcmp(cls->ptr, variants[index].key, variants[index].key_len) == 0) {
			const keymap_t *mapping = NULL;
			for (uint8_t ind = 0; ind < sizeof(mappings) / sizeof(keymap_t); ind++) {
				if (cls->len - variants[index].key_len == mappings[ind].key_len &&
						memcmp(&cls->ptr[variants[index].key_len], mappings[ind].key, mappings[ind].key_len) == 0) {
					mapping = &mappings[ind];
					break;
				}
			}
			if (mapping == NULL) {
				return;
			}

			cls->known = stamp(pfx, cls, &variants[index], mapping, NULL, buffer, buffer_len);
		}
	}
}

void flex(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint) {
	if (pfx->len != breakpoint->tag_len || memcmp(pfx->ptr, breakpoint->tag, breakpoint->tag_len) != 0) {
		return;
	}

	const keymap_t flexes[] = {
			{.key = "flex-1", .key_len = 6, .val = "flex:1 1 0%", .val_len = 11},
			{.key = "flex-auto", .key_len = 9, .val = "flex:1 1 auto", .val_len = 13},
			{.key = "flex-initial", .key_len = 12, .val = "flex:0 1 auto", .val_len = 13},
			{.key = "flex-none", .key_len = 9, .val = "flex:none", .val_len = 9},
			{.key = "flex-row", .key_len = 8, .val = "flex-direction:row", .val_len = 18},
			{.key = "flex-row-reverse", .key_len = 16, .val = "flex-direction:row-reverse", .val_len = 26},
			{.key = "flex-col", .key_len = 8, .val = "flex-direction:column", .val_len = 21},
			{.key = "flex-col-reverse", .key_len = 16, .val = "flex-direction:column-reverse", .val_len = 29},
			{.key = "flex-wrap", .key_len = 9, .val = "flex-wrap:wrap", .val_len = 14},
			{.key = "flex-wrap-reverse", .key_len = 17, .val = "flex-wrap:wrap-reverse", .val_len = 22},
			{.key = "flex-nowrap", .key_len = 11, .val = "flex-wrap:nowrap", .val_len = 16},
			{.key = "justify-normal", .key_len = 14, .val = "justify-content:normal", .val_len = 23},
			{.key = "justify-start", .key_len = 13, .val = "justify-content:flex-start", .val_len = 26},
			{.key = "justify-end", .key_len = 11, .val = "justify-content:flex-end", .val_len = 24},
			{.key = "justify-center", .key_len = 14, .val = "justify-content:center", .val_len = 22},
			{.key = "justify-between", .key_len = 15, .val = "justify-content:space-between", .val_len = 29},
			{.key = "justify-around", .key_len = 14, .val = "justify-content:space-around", .val_len = 28},
			{.key = "justify-evenly", .key_len = 14, .val = "justify-content:space-evenly", .val_len = 28},
			{.key = "justify-stretch", .key_len = 15, .val = "justify-content:stretch", .val_len = 23},
			{.key = "items-start", .key_len = 11, .val = "align-items:flex-start", .val_len = 22},
			{.key = "items-end", .key_len = 9, .val = "align-items:flex-end", .val_len = 20},
			{.key = "items-center", .key_len = 12, .val = "align-items:center", .val_len = 18},
			{.key = "items-baseline", .key_len = 14, .val = "align-items:baseline", .val_len = 20},
			{.key = "items-stretch", .key_len = 13, .val = "align-items:stretch", .val_len = 19},
			{.key = "grow", .key_len = 4, .val = "flex-grow:1", .val_len = 11},
			{.key = "grow-0", .key_len = 6, .val = "flex-grow:0", .val_len = 11},
			{.key = "shrink", .key_len = 6, .val = "flex-shrink:1", .val_len = 13},
			{.key = "shrink-0", .key_len = 8, .val = "flex-shrink:0", .val_len = 13},
	};

	for (uint8_t index = 0; index < sizeof(flexes) / sizeof(keymap_t); index++) {
		if (cls->len == flexes[index].key_len && memcmp(cls->ptr, flexes[index].key, flexes[index].key_len) == 0) {
			cls->known = stamp(pfx, cls, &flexes[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void text(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t texts[] = {
			{.key = "text-xs", .key_len = 7, .val = "font-size:12px", .val_len = 14},
			{.key = "text-sm", .key_len = 7, .val = "font-size:14px", .val_len = 14},
			{.key = "text-base", .key_len = 9, .val = "font-size:16px", .val_len = 14},
			{.key = "text-lg", .key_len = 7, .val = "font-size:18px", .val_len = 14},
			{.key = "text-xl", .key_len = 7, .val = "font-size:20px", .val_len = 14},
			{.key = "text-2xl", .key_len = 8, .val = "font-size:24px", .val_len = 14},
			{.key = "text-3xl", .key_len = 8, .val = "font-size:30px", .val_len = 14},
			{.key = "text-4xl", .key_len = 8, .val = "font-size:36px", .val_len = 14},
			{.key = "text-left", .key_len = 9, .val = "text-align:left", .val_len = 0},
			{.key = "text-center", .key_len = 11, .val = "text-align:center", .val_len = 17},
			{.key = "text-right", .key_len = 10, .val = "text-align:right", .val_len = 16},
			{.key = "text-justify", .key_len = 12, .val = "text-align:justify", .val_len = 18},
			{.key = "text-start", .key_len = 10, .val = "text-align:start", .val_len = 16},
			{.key = "text-end", .key_len = 8, .val = "text-align:end", .val_len = 14},
			{.key = "text-wrap", .key_len = 9, .val = "text-wrap:wrap", .val_len = 14},
			{.key = "text-nowrap", .key_len = 11, .val = "text-wrap:nowrap", .val_len = 16},
			{.key = "text-balance", .key_len = 12, .val = "text-wrap:balance", .val_len = 17},
			{.key = "text-pretty", .key_len = 11, .val = "text-wrap:pretty", .val_len = 16},
			{.key = "leading-none", .key_len = 12, .val = "line-height:1", .val_len = 13},
			{.key = "leading-slim", .key_len = 12, .val = "line-height:1.125", .val_len = 17},
			{.key = "leading-tight", .key_len = 13, .val = "line-height:1.25", .val_len = 16},
			{.key = "leading-snug", .key_len = 12, .val = "line-height:1.375", .val_len = 17},
			{.key = "leading-normal", .key_len = 14, .val = "line-height:1.5", .val_len = 15},
			{.key = "leading-relaxed", .key_len = 15, .val = "line-height:1.625", .val_len = 17},
			{.key = "leading-loose", .key_len = 13, .val = "line-height:2", .val_len = 13},
			{.key = "underline", .key_len = 9, .val = "text-decoration-line:underline", .val_len = 30},
			{.key = "overline", .key_len = 8, .val = "text-decoration-line:overline", .val_len = 29},
			{.key = "line-through", .key_len = 12, .val = "text-decoration-line:line-through", .val_len = 33},
			{.key = "no-underline", .key_len = 12, .val = "text-decoration-line:none", .val_len = 25},
	};

	for (uint8_t index = 0; index < sizeof(texts) / sizeof(keymap_t); index++) {
		if (cls->len == texts[index].key_len && memcmp(cls->ptr, texts[index].key, texts[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &texts[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void font(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t fonts[] = {
			{.key = "font-thin", .key_len = 9, .val = "font-weight:100", .val_len = 15},
			{.key = "font-extralight", .key_len = 15, .val = "font-weight:200", .val_len = 15},
			{.key = "font-light", .key_len = 10, .val = "font-weight:300", .val_len = 15},
			{.key = "font-normal", .key_len = 11, .val = "font-weight:400", .val_len = 15},
			{.key = "font-medium", .key_len = 11, .val = "font-weight:500", .val_len = 15},
			{.key = "font-semibold", .key_len = 13, .val = "font-weight:600", .val_len = 15},
			{.key = "font-bold", .key_len = 10, .val = "font-weight:700", .val_len = 15},
			{.key = "font-extrabold", .key_len = 14, .val = "font-weight:800", .val_len = 15},
			{.key = "font-black", .key_len = 11, .val = "font-weight:900", .val_len = 15},
	};

	for (uint8_t index = 0; index < sizeof(fonts) / sizeof(keymap_t); index++) {
		if (cls->len == fonts[index].key_len && memcmp(cls->ptr, fonts[index].key, fonts[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &fonts[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void color(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint) {
	if (pfx->len != breakpoint->tag_len || memcmp(pfx->ptr, breakpoint->tag, breakpoint->tag_len) != 0) {
		return;
	}

	const keymap_t variants[] = {
			{.key = "text-", .key_len = 5, .val = "color", .val_len = 5},
			{.key = "stroke-", .key_len = 7, .val = "stroke", .val_len = 6},
			{.key = "border-t-", .key_len = 9, .val = "border-top-color", .val_len = 16},
			{.key = "border-r-", .key_len = 9, .val = "border-right-color", .val_len = 18},
			{.key = "border-b-", .key_len = 9, .val = "border-bottom-color", .val_len = 19},
			{.key = "border-l-", .key_len = 9, .val = "border-left-color", .val_len = 17},
			{.key = "border-", .key_len = 7, .val = "border-color", .val_len = 12},
			{.key = "background-", .key_len = 11, .val = "background-color", .val_len = 16},
	};

	const keymap_t mappings[] = {
			{.key = "inherit", .key_len = 7, .val = "inherit", .val_len = 7},
			{.key = "transparent", .key_len = 11, .val = "transparent", .val_len = 11},
			{.key = "white", .key_len = 5, .val = "#ffffff", .val_len = 7},
			{.key = "black", .key_len = 5, .val = "#000000", .val_len = 7},
			{.key = "slate-50", .key_len = 8, .val = "#f8fafc", .val_len = 7},
			{.key = "slate-100", .key_len = 9, .val = "#f1f5f9", .val_len = 7},
			{.key = "slate-200", .key_len = 9, .val = "#e2e8f0", .val_len = 7},
			{.key = "slate-300", .key_len = 9, .val = "#cbd5e1", .val_len = 7},
			{.key = "slate-400", .key_len = 9, .val = "#94a3b8", .val_len = 7},
			{.key = "slate-500", .key_len = 9, .val = "#64748b", .val_len = 7},
			{.key = "slate-600", .key_len = 9, .val = "#475569", .val_len = 7},
			{.key = "slate-700", .key_len = 9, .val = "#334155", .val_len = 7},
			{.key = "slate-800", .key_len = 9, .val = "#1e293b", .val_len = 7},
			{.key = "slate-900", .key_len = 9, .val = "#0f172a", .val_len = 7},
			{.key = "slate-950", .key_len = 9, .val = "#020617", .val_len = 7},
			{.key = "gray-50", .key_len = 7, .val = "#f9fafb", .val_len = 7},
			{.key = "gray-100", .key_len = 8, .val = "#f3f4f6", .val_len = 7},
			{.key = "gray-200", .key_len = 8, .val = "#e5e7eb", .val_len = 7},
			{.key = "gray-300", .key_len = 8, .val = "#d1d5db", .val_len = 7},
			{.key = "gray-400", .key_len = 8, .val = "#9ca3af", .val_len = 7},
			{.key = "gray-500", .key_len = 8, .val = "#6b7280", .val_len = 7},
			{.key = "gray-600", .key_len = 8, .val = "#4b5563", .val_len = 7},
			{.key = "gray-700", .key_len = 8, .val = "#374151", .val_len = 7},
			{.key = "gray-800", .key_len = 8, .val = "#1f2937", .val_len = 7},
			{.key = "gray-900", .key_len = 8, .val = "#111827", .val_len = 7},
			{.key = "gray-950", .key_len = 8, .val = "#030712", .val_len = 7},
			{.key = "zinc-50", .key_len = 7, .val = "#fafafa", .val_len = 7},
			{.key = "zinc-100", .key_len = 8, .val = "#f4f4f5", .val_len = 7},
			{.key = "zinc-200", .key_len = 8, .val = "#e4e4e7", .val_len = 7},
			{.key = "zinc-300", .key_len = 8, .val = "#d4d4d8", .val_len = 7},
			{.key = "zinc-400", .key_len = 8, .val = "#a1a1aa", .val_len = 7},
			{.key = "zinc-500", .key_len = 8, .val = "#71717a", .val_len = 7},
			{.key = "zinc-600", .key_len = 8, .val = "#52525b", .val_len = 7},
			{.key = "zinc-700", .key_len = 8, .val = "#3f3f46", .val_len = 7},
			{.key = "zinc-800", .key_len = 8, .val = "#27272a", .val_len = 7},
			{.key = "zinc-900", .key_len = 8, .val = "#18181b", .val_len = 7},
			{.key = "zinc-950", .key_len = 8, .val = "#09090b", .val_len = 7},
			{.key = "neutral-50", .key_len = 10, .val = "#fafafa", .val_len = 7},
			{.key = "neutral-100", .key_len = 11, .val = "#f5f5f5", .val_len = 7},
			{.key = "neutral-200", .key_len = 11, .val = "#e5e5e5", .val_len = 7},
			{.key = "neutral-300", .key_len = 11, .val = "#d4d4d4", .val_len = 7},
			{.key = "neutral-400", .key_len = 11, .val = "#a3a3a3", .val_len = 7},
			{.key = "neutral-500", .key_len = 11, .val = "#737373", .val_len = 7},
			{.key = "neutral-600", .key_len = 11, .val = "#525252", .val_len = 7},
			{.key = "neutral-700", .key_len = 11, .val = "#404040", .val_len = 7},
			{.key = "neutral-800", .key_len = 11, .val = "#262626", .val_len = 7},
			{.key = "neutral-900", .key_len = 11, .val = "#171717", .val_len = 7},
			{.key = "neutral-950", .key_len = 11, .val = "#0a0a0a", .val_len = 7},
			{.key = "stone-50", .key_len = 8, .val = "#fafaf9", .val_len = 7},
			{.key = "stone-100", .key_len = 9, .val = "#f5f5f4", .val_len = 7},
			{.key = "stone-200", .key_len = 9, .val = "#e7e5e4", .val_len = 7},
			{.key = "stone-300", .key_len = 9, .val = "#d6d3d1", .val_len = 7},
			{.key = "stone-400", .key_len = 9, .val = "#a8a29e", .val_len = 7},
			{.key = "stone-500", .key_len = 9, .val = "#78716c", .val_len = 7},
			{.key = "stone-600", .key_len = 9, .val = "#57534e", .val_len = 7},
			{.key = "stone-700", .key_len = 9, .val = "#44403c", .val_len = 7},
			{.key = "stone-800", .key_len = 9, .val = "#292524", .val_len = 7},
			{.key = "stone-900", .key_len = 9, .val = "#1c1917", .val_len = 7},
			{.key = "stone-950", .key_len = 9, .val = "#0c0a09", .val_len = 7},
			{.key = "red-50", .key_len = 6, .val = "#fef2f2", .val_len = 7},
			{.key = "red-100", .key_len = 7, .val = "#fee2e2", .val_len = 7},
			{.key = "red-200", .key_len = 7, .val = "#fecaca", .val_len = 7},
			{.key = "red-300", .key_len = 7, .val = "#fca5a5", .val_len = 7},
			{.key = "red-400", .key_len = 7, .val = "#f87171", .val_len = 7},
			{.key = "red-500", .key_len = 7, .val = "#ef4444", .val_len = 7},
			{.key = "red-600", .key_len = 7, .val = "#dc2626", .val_len = 7},
			{.key = "red-700", .key_len = 7, .val = "#b91c1c", .val_len = 7},
			{.key = "red-800", .key_len = 7, .val = "#991b1b", .val_len = 7},
			{.key = "red-900", .key_len = 7, .val = "#7f1d1d", .val_len = 7},
			{.key = "red-950", .key_len = 7, .val = "#450a0a", .val_len = 7},
			{.key = "orange-50", .key_len = 9, .val = "#fff7ed", .val_len = 7},
			{.key = "orange-100", .key_len = 10, .val = "#ffedd5", .val_len = 7},
			{.key = "orange-200", .key_len = 10, .val = "#fed7aa", .val_len = 7},
			{.key = "orange-300", .key_len = 10, .val = "#fdba74", .val_len = 7},
			{.key = "orange-400", .key_len = 10, .val = "#fb923c", .val_len = 7},
			{.key = "orange-500", .key_len = 10, .val = "#f97316", .val_len = 7},
			{.key = "orange-600", .key_len = 10, .val = "#ea580c", .val_len = 7},
			{.key = "orange-700", .key_len = 10, .val = "#c2410c", .val_len = 7},
			{.key = "orange-800", .key_len = 10, .val = "#9a3412", .val_len = 7},
			{.key = "orange-900", .key_len = 10, .val = "#7c2d12", .val_len = 7},
			{.key = "orange-950", .key_len = 10, .val = "#431407", .val_len = 7},
			{.key = "amber-50", .key_len = 8, .val = "#fffbeb", .val_len = 7},
			{.key = "amber-100", .key_len = 9, .val = "#fef3c7", .val_len = 7},
			{.key = "amber-200", .key_len = 9, .val = "#fde68a", .val_len = 7},
			{.key = "amber-300", .key_len = 9, .val = "#fcd34d", .val_len = 7},
			{.key = "amber-400", .key_len = 9, .val = "#fbbf24", .val_len = 7},
			{.key = "amber-500", .key_len = 9, .val = "#f59e0b", .val_len = 7},
			{.key = "amber-600", .key_len = 9, .val = "#d97706", .val_len = 7},
			{.key = "amber-700", .key_len = 9, .val = "#b45309", .val_len = 7},
			{.key = "amber-800", .key_len = 9, .val = "#92400e", .val_len = 7},
			{.key = "amber-900", .key_len = 9, .val = "#78350f", .val_len = 7},
			{.key = "amber-950", .key_len = 9, .val = "#451a03", .val_len = 7},
			{.key = "yellow-50", .key_len = 9, .val = "#fefce8", .val_len = 7},
			{.key = "yellow-100", .key_len = 10, .val = "#fef9c3", .val_len = 7},
			{.key = "yellow-200", .key_len = 10, .val = "#fef08a", .val_len = 7},
			{.key = "yellow-300", .key_len = 10, .val = "#fde047", .val_len = 7},
			{.key = "yellow-400", .key_len = 10, .val = "#facc15", .val_len = 7},
			{.key = "yellow-500", .key_len = 10, .val = "#eab308", .val_len = 7},
			{.key = "yellow-600", .key_len = 10, .val = "#ca8a04", .val_len = 7},
			{.key = "yellow-700", .key_len = 10, .val = "#a16207", .val_len = 7},
			{.key = "yellow-800", .key_len = 10, .val = "#854d0e", .val_len = 7},
			{.key = "yellow-900", .key_len = 10, .val = "#713f12", .val_len = 7},
			{.key = "yellow-950", .key_len = 10, .val = "#422006", .val_len = 7},
			{.key = "lime-50", .key_len = 7, .val = "#f7fee7", .val_len = 7},
			{.key = "lime-100", .key_len = 8, .val = "#ecfccb", .val_len = 7},
			{.key = "lime-200", .key_len = 8, .val = "#d9f99d", .val_len = 7},
			{.key = "lime-300", .key_len = 8, .val = "#bef264", .val_len = 7},
			{.key = "lime-400", .key_len = 8, .val = "#a3e635", .val_len = 7},
			{.key = "lime-500", .key_len = 8, .val = "#84cc16", .val_len = 7},
			{.key = "lime-600", .key_len = 8, .val = "#65a30d", .val_len = 7},
			{.key = "lime-700", .key_len = 8, .val = "#4d7c0f", .val_len = 7},
			{.key = "lime-800", .key_len = 8, .val = "#3f6212", .val_len = 7},
			{.key = "lime-900", .key_len = 8, .val = "#365314", .val_len = 7},
			{.key = "lime-950", .key_len = 8, .val = "#1a2e05", .val_len = 7},
			{.key = "green-50", .key_len = 8, .val = "#f0fdf4", .val_len = 7},
			{.key = "green-100", .key_len = 9, .val = "#dcfce7", .val_len = 7},
			{.key = "green-200", .key_len = 9, .val = "#bbf7d0", .val_len = 7},
			{.key = "green-300", .key_len = 9, .val = "#86efac", .val_len = 7},
			{.key = "green-400", .key_len = 9, .val = "#4ade80", .val_len = 7},
			{.key = "green-500", .key_len = 9, .val = "#22c55e", .val_len = 7},
			{.key = "green-600", .key_len = 9, .val = "#16a34a", .val_len = 7},
			{.key = "green-700", .key_len = 9, .val = "#15803d", .val_len = 7},
			{.key = "green-800", .key_len = 9, .val = "#166534", .val_len = 7},
			{.key = "green-900", .key_len = 9, .val = "#14532d", .val_len = 7},
			{.key = "green-950", .key_len = 9, .val = "#052e16", .val_len = 7},
			{.key = "emerald-50", .key_len = 10, .val = "#ecfdf5", .val_len = 7},
			{.key = "emerald-100", .key_len = 11, .val = "#d1fae5", .val_len = 7},
			{.key = "emerald-200", .key_len = 11, .val = "#a7f3d0", .val_len = 7},
			{.key = "emerald-300", .key_len = 11, .val = "#6ee7b7", .val_len = 7},
			{.key = "emerald-400", .key_len = 11, .val = "#34d399", .val_len = 7},
			{.key = "emerald-500", .key_len = 11, .val = "#10b981", .val_len = 7},
			{.key = "emerald-600", .key_len = 11, .val = "#059669", .val_len = 7},
			{.key = "emerald-700", .key_len = 11, .val = "#047857", .val_len = 7},
			{.key = "emerald-800", .key_len = 11, .val = "#065f46", .val_len = 7},
			{.key = "emerald-900", .key_len = 11, .val = "#064e3b", .val_len = 7},
			{.key = "emerald-950", .key_len = 11, .val = "#022c22", .val_len = 7},
			{.key = "teal-50", .key_len = 7, .val = "#f0fdfa", .val_len = 7},
			{.key = "teal-100", .key_len = 8, .val = "#ccfbf1", .val_len = 7},
			{.key = "teal-200", .key_len = 8, .val = "#99f6e4", .val_len = 7},
			{.key = "teal-300", .key_len = 8, .val = "#5eead4", .val_len = 7},
			{.key = "teal-400", .key_len = 8, .val = "#2dd4bf", .val_len = 7},
			{.key = "teal-500", .key_len = 8, .val = "#14b8a6", .val_len = 7},
			{.key = "teal-600", .key_len = 8, .val = "#0d9488", .val_len = 7},
			{.key = "teal-700", .key_len = 8, .val = "#0f766e", .val_len = 7},
			{.key = "teal-800", .key_len = 8, .val = "#115e59", .val_len = 7},
			{.key = "teal-900", .key_len = 8, .val = "#134e4a", .val_len = 7},
			{.key = "teal-950", .key_len = 8, .val = "#042f2e", .val_len = 7},
			{.key = "cyan-50", .key_len = 7, .val = "#ecfeff", .val_len = 7},
			{.key = "cyan-100", .key_len = 8, .val = "#cffafe", .val_len = 7},
			{.key = "cyan-200", .key_len = 8, .val = "#a5f3fc", .val_len = 7},
			{.key = "cyan-300", .key_len = 8, .val = "#67e8f9", .val_len = 7},
			{.key = "cyan-400", .key_len = 8, .val = "#22d3ee", .val_len = 7},
			{.key = "cyan-500", .key_len = 8, .val = "#06b6d4", .val_len = 7},
			{.key = "cyan-600", .key_len = 8, .val = "#0891b2", .val_len = 7},
			{.key = "cyan-700", .key_len = 8, .val = "#0e7490", .val_len = 7},
			{.key = "cyan-800", .key_len = 8, .val = "#155e75", .val_len = 7},
			{.key = "cyan-900", .key_len = 8, .val = "#164e63", .val_len = 7},
			{.key = "cyan-950", .key_len = 8, .val = "#083344", .val_len = 7},
			{.key = "sky-50", .key_len = 6, .val = "#f0f9ff", .val_len = 7},
			{.key = "sky-100", .key_len = 7, .val = "#e0f2fe", .val_len = 7},
			{.key = "sky-200", .key_len = 7, .val = "#bae6fd", .val_len = 7},
			{.key = "sky-300", .key_len = 7, .val = "#7dd3fc", .val_len = 7},
			{.key = "sky-400", .key_len = 7, .val = "#38bdf8", .val_len = 7},
			{.key = "sky-500", .key_len = 7, .val = "#0ea5e9", .val_len = 7},
			{.key = "sky-600", .key_len = 7, .val = "#0284c7", .val_len = 7},
			{.key = "sky-700", .key_len = 7, .val = "#0369a1", .val_len = 7},
			{.key = "sky-800", .key_len = 7, .val = "#075985", .val_len = 7},
			{.key = "sky-900", .key_len = 7, .val = "#0c4a6e", .val_len = 7},
			{.key = "sky-950", .key_len = 7, .val = "#082f49", .val_len = 7},
			{.key = "blue-50", .key_len = 7, .val = "#eff6ff", .val_len = 7},
			{.key = "blue-100", .key_len = 8, .val = "#dbeafe", .val_len = 7},
			{.key = "blue-200", .key_len = 8, .val = "#bfdbfe", .val_len = 7},
			{.key = "blue-300", .key_len = 8, .val = "#93c5fd", .val_len = 7},
			{.key = "blue-400", .key_len = 8, .val = "#60a5fa", .val_len = 7},
			{.key = "blue-500", .key_len = 8, .val = "#3b82f6", .val_len = 7},
			{.key = "blue-600", .key_len = 8, .val = "#2563eb", .val_len = 7},
			{.key = "blue-700", .key_len = 8, .val = "#1d4ed8", .val_len = 7},
			{.key = "blue-800", .key_len = 8, .val = "#1e40af", .val_len = 7},
			{.key = "blue-900", .key_len = 8, .val = "#1e3a8a", .val_len = 7},
			{.key = "blue-950", .key_len = 8, .val = "#172554", .val_len = 7},
			{.key = "indigo-50", .key_len = 9, .val = "#eef2ff", .val_len = 7},
			{.key = "indigo-100", .key_len = 10, .val = "#e0e7ff", .val_len = 7},
			{.key = "indigo-200", .key_len = 10, .val = "#c7d2fe", .val_len = 7},
			{.key = "indigo-300", .key_len = 10, .val = "#a5b4fc", .val_len = 7},
			{.key = "indigo-400", .key_len = 10, .val = "#818cf8", .val_len = 7},
			{.key = "indigo-500", .key_len = 10, .val = "#6366f1", .val_len = 7},
			{.key = "indigo-600", .key_len = 10, .val = "#4f46e5", .val_len = 7},
			{.key = "indigo-700", .key_len = 10, .val = "#4338ca", .val_len = 7},
			{.key = "indigo-800", .key_len = 10, .val = "#3730a3", .val_len = 7},
			{.key = "indigo-900", .key_len = 10, .val = "#312e81", .val_len = 7},
			{.key = "indigo-950", .key_len = 10, .val = "#1e1b4b", .val_len = 7},
			{.key = "violet-50", .key_len = 9, .val = "#f5f3ff", .val_len = 7},
			{.key = "violet-100", .key_len = 10, .val = "#ede9fe", .val_len = 7},
			{.key = "violet-200", .key_len = 10, .val = "#ddd6fe", .val_len = 7},
			{.key = "violet-300", .key_len = 10, .val = "#c4b5fd", .val_len = 7},
			{.key = "violet-400", .key_len = 10, .val = "#a78bfa", .val_len = 7},
			{.key = "violet-500", .key_len = 10, .val = "#8b5cf6", .val_len = 7},
			{.key = "violet-600", .key_len = 10, .val = "#7c3aed", .val_len = 7},
			{.key = "violet-700", .key_len = 10, .val = "#6d28d9", .val_len = 7},
			{.key = "violet-800", .key_len = 10, .val = "#5b21b6", .val_len = 7},
			{.key = "violet-900", .key_len = 10, .val = "#4c1d95", .val_len = 7},
			{.key = "violet-950", .key_len = 10, .val = "#2e1065", .val_len = 7},
			{.key = "purple-50", .key_len = 9, .val = "#faf5ff", .val_len = 7},
			{.key = "purple-100", .key_len = 10, .val = "#f3e8ff", .val_len = 7},
			{.key = "purple-200", .key_len = 10, .val = "#e9d5ff", .val_len = 7},
			{.key = "purple-300", .key_len = 10, .val = "#d8b4fe", .val_len = 7},
			{.key = "purple-400", .key_len = 10, .val = "#c084fc", .val_len = 7},
			{.key = "purple-500", .key_len = 10, .val = "#a855f7", .val_len = 7},
			{.key = "purple-600", .key_len = 10, .val = "#9333ea", .val_len = 7},
			{.key = "purple-700", .key_len = 10, .val = "#7e22ce", .val_len = 7},
			{.key = "purple-800", .key_len = 10, .val = "#6b21a8", .val_len = 7},
			{.key = "purple-900", .key_len = 10, .val = "#581c87", .val_len = 7},
			{.key = "purple-950", .key_len = 10, .val = "#3b0764", .val_len = 7},
			{.key = "fuchsia-50", .key_len = 10, .val = "#fdf4ff", .val_len = 7},
			{.key = "fuchsia-100", .key_len = 11, .val = "#fae8ff", .val_len = 7},
			{.key = "fuchsia-200", .key_len = 11, .val = "#f5d0fe", .val_len = 7},
			{.key = "fuchsia-300", .key_len = 11, .val = "#f0abfc", .val_len = 7},
			{.key = "fuchsia-400", .key_len = 11, .val = "#e879f9", .val_len = 7},
			{.key = "fuchsia-500", .key_len = 11, .val = "#d946ef", .val_len = 7},
			{.key = "fuchsia-600", .key_len = 11, .val = "#c026d3", .val_len = 7},
			{.key = "fuchsia-700", .key_len = 11, .val = "#a21caf", .val_len = 7},
			{.key = "fuchsia-800", .key_len = 11, .val = "#86198f", .val_len = 7},
			{.key = "fuchsia-900", .key_len = 11, .val = "#701a75", .val_len = 7},
			{.key = "fuchsia-950", .key_len = 11, .val = "#4a044e", .val_len = 7},
			{.key = "pink-50", .key_len = 7, .val = "#fdf2f8", .val_len = 7},
			{.key = "pink-100", .key_len = 8, .val = "#fce7f3", .val_len = 7},
			{.key = "pink-200", .key_len = 8, .val = "#fbcfe8", .val_len = 7},
			{.key = "pink-300", .key_len = 8, .val = "#f9a8d4", .val_len = 7},
			{.key = "pink-400", .key_len = 8, .val = "#f472b6", .val_len = 7},
			{.key = "pink-500", .key_len = 8, .val = "#ec4899", .val_len = 7},
			{.key = "pink-600", .key_len = 8, .val = "#db2777", .val_len = 7},
			{.key = "pink-700", .key_len = 8, .val = "#be185d", .val_len = 7},
			{.key = "pink-800", .key_len = 8, .val = "#9d174d", .val_len = 7},
			{.key = "pink-900", .key_len = 8, .val = "#831843", .val_len = 7},
			{.key = "pink-950", .key_len = 8, .val = "#500724", .val_len = 7},
			{.key = "rose-50", .key_len = 7, .val = "#fff1f2", .val_len = 7},
			{.key = "rose-100", .key_len = 8, .val = "#ffe4e6", .val_len = 7},
			{.key = "rose-200", .key_len = 8, .val = "#fecdd3", .val_len = 7},
			{.key = "rose-300", .key_len = 8, .val = "#fda4af", .val_len = 7},
			{.key = "rose-400", .key_len = 8, .val = "#fb7185", .val_len = 7},
			{.key = "rose-500", .key_len = 8, .val = "#f43f5e", .val_len = 7},
			{.key = "rose-600", .key_len = 8, .val = "#e11d48", .val_len = 7},
			{.key = "rose-700", .key_len = 8, .val = "#be123c", .val_len = 7},
			{.key = "rose-800", .key_len = 8, .val = "#9f1239", .val_len = 7},
			{.key = "rose-900", .key_len = 8, .val = "#881337", .val_len = 7},
			{.key = "rose-950", .key_len = 8, .val = "#4c0519", .val_len = 7},
	};

	const keymap_t alphas[] = {
			{.key = "/5", .key_len = 2, .val = "0d", .val_len = 2},	 {.key = "/10", .key_len = 3, .val = "1a", .val_len = 2},
			{.key = "/15", .key_len = 3, .val = "26", .val_len = 2}, {.key = "/20", .key_len = 3, .val = "33", .val_len = 2},
			{.key = "/25", .key_len = 3, .val = "40", .val_len = 2}, {.key = "/30", .key_len = 3, .val = "4d", .val_len = 2},
			{.key = "/35", .key_len = 3, .val = "59", .val_len = 2}, {.key = "/40", .key_len = 3, .val = "66", .val_len = 2},
			{.key = "/45", .key_len = 3, .val = "73", .val_len = 2}, {.key = "/50", .key_len = 3, .val = "80", .val_len = 2},
			{.key = "/55", .key_len = 3, .val = "8c", .val_len = 2}, {.key = "/60", .key_len = 3, .val = "99", .val_len = 2},
			{.key = "/65", .key_len = 3, .val = "a6", .val_len = 2}, {.key = "/70", .key_len = 3, .val = "b3", .val_len = 2},
			{.key = "/75", .key_len = 3, .val = "bf", .val_len = 2}, {.key = "/80", .key_len = 3, .val = "cc", .val_len = 2},
			{.key = "/85", .key_len = 3, .val = "d9", .val_len = 2}, {.key = "/90", .key_len = 3, .val = "e6", .val_len = 2},
			{.key = "/95", .key_len = 3, .val = "f2", .val_len = 2},
	};

	for (uint8_t index = 0; index < sizeof(variants) / sizeof(keymap_t); index++) {
		if (cls->len > variants[index].key_len && memcmp(cls->ptr, variants[index].key, variants[index].key_len) == 0) {
			const keymap_t *mapping = NULL;
			const keymap_t *alpha = NULL;
			for (uint8_t ind = 0; ind < sizeof(mappings) / sizeof(keymap_t); ind++) {
				if (cls->len - variants[index].key_len >= mappings[ind].key_len &&
						memcmp(&cls->ptr[variants[index].key_len], mappings[ind].key, mappings[ind].key_len) == 0) {
					for (uint8_t i = 0; i < sizeof(alphas) / sizeof(keymap_t); i++) {
						if (cls->len - variants[index].key_len - mappings[ind].key_len == alphas[i].key_len &&
								memcmp(&cls->ptr[variants[index].key_len + mappings[ind].key_len], alphas[i].key, alphas[i].key_len) == 0) {
							alpha = &alphas[i];
							break;
						}
					}
					mapping = &mappings[ind];
					break;
				}
			}
			if (mapping == NULL) {
				return;
			}
			if (cls->len > variants[index].key_len + mapping->key_len && alpha == NULL) {
				return;
			}

			cls->known = stamp(pfx, cls, &variants[index], mapping, alpha, buffer, buffer_len);
		}
	}
}

void opacity(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t opacities[] = {
			{.key = "opacity-0", .key_len = 9, .val = "opacity:0", .val_len = 9},
			{.key = "opacity-5", .key_len = 9, .val = "opacity:0.05", .val_len = 12},
			{.key = "opacity-10", .key_len = 10, .val = "opacity:0.1", .val_len = 11},
			{.key = "opacity-15", .key_len = 10, .val = "opacity:0.15", .val_len = 12},
			{.key = "opacity-20", .key_len = 10, .val = "opacity:0.2", .val_len = 11},
			{.key = "opacity-25", .key_len = 10, .val = "opacity:0.25", .val_len = 12},
			{.key = "opacity-30", .key_len = 10, .val = "opacity:0.3", .val_len = 11},
			{.key = "opacity-35", .key_len = 10, .val = "opacity:0.35", .val_len = 12},
			{.key = "opacity-40", .key_len = 10, .val = "opacity:0.4", .val_len = 11},
			{.key = "opacity-45", .key_len = 10, .val = "opacity:0.45", .val_len = 12},
			{.key = "opacity-50", .key_len = 10, .val = "opacity:0.5", .val_len = 11},
			{.key = "opacity-55", .key_len = 10, .val = "opacity:0.55", .val_len = 12},
			{.key = "opacity-60", .key_len = 10, .val = "opacity:0.6", .val_len = 11},
			{.key = "opacity-65", .key_len = 10, .val = "opacity:0.65", .val_len = 12},
			{.key = "opacity-70", .key_len = 10, .val = "opacity:0.7", .val_len = 11},
			{.key = "opacity-75", .key_len = 10, .val = "opacity:0.75", .val_len = 12},
			{.key = "opacity-80", .key_len = 10, .val = "opacity:0.8", .val_len = 11},
			{.key = "opacity-85", .key_len = 10, .val = "opacity:0.85", .val_len = 12},
			{.key = "opacity-90", .key_len = 10, .val = "opacity:0.9", .val_len = 11},
			{.key = "opacity-95", .key_len = 10, .val = "opacity:0.95", .val_len = 12},
			{.key = "opacity-100", .key_len = 11, .val = "opacity:1", .val_len = 9},
	};

	for (uint8_t index = 0; index < sizeof(opacities) / sizeof(keymap_t); index++) {
		if (cls->len == opacities[index].key_len && memcmp(cls->ptr, opacities[index].key, opacities[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &opacities[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void cursor(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t texts[] = {
			{.key = "cursor-auto", .key_len = 11, .val = "cursor:auto", .val_len = 11},
			{.key = "cursor-default", .key_len = 14, .val = "cursor:default", .val_len = 14},
			{.key = "cursor-pointer", .key_len = 14, .val = "cursor:pointer", .val_len = 14},
			{.key = "cursor-wait", .key_len = 11, .val = "cursor:wait", .val_len = 11},
			{.key = "cursor-text", .key_len = 11, .val = "cursor:text", .val_len = 11},
			{.key = "cursor-move", .key_len = 11, .val = "cursor:move", .val_len = 11},
			{.key = "cursor-help", .key_len = 11, .val = "cursor:help", .val_len = 11},
			{.key = "cursor-not-allowed", .key_len = 18, .val = "cursor:not-allowed", .val_len = 18},
			{.key = "cursor-none", .key_len = 11, .val = "cursor-none", .val_len = 11},
			{.key = "cursor-context-menu", .key_len = 19, .val = "cursor:context-menu", .val_len = 19},
			{.key = "cursor-progress", .key_len = 15, .val = "cursor:progress", .val_len = 15},
			{.key = "cursor-cell", .key_len = 11, .val = "cursor:cell", .val_len = 11},
			{.key = "cursor-crosshair", .key_len = 16, .val = "cursor:crosshair", .val_len = 16},
			{.key = "cursor-vertical-text", .key_len = 20, .val = "cursor:vertical-text", .val_len = 20},
			{.key = "cursor-alias", .key_len = 12, .val = "cursor:alias", .val_len = 12},
			{.key = "cursor-copy", .key_len = 11, .val = "cursor:copy", .val_len = 11},
			{.key = "cursor-no-drop", .key_len = 14, .val = "cursor:no-drop", .val_len = 14},
			{.key = "cursor-grab", .key_len = 11, .val = "cursor:grab", .val_len = 11},
			{.key = "cursor-grabbing", .key_len = 15, .val = "cursor:grabbing", .val_len = 15},
	};

	for (uint8_t index = 0; index < sizeof(texts) / sizeof(keymap_t); index++) {
		if (cls->len == texts[index].key_len && memcmp(cls->ptr, texts[index].key, texts[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &texts[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void layout(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t layouts[] = {
			{.key = "z-0", .key_len = 3, .val = "z-index:0", .val_len = 9},
			{.key = "z-4", .key_len = 3, .val = "z-index:4", .val_len = 9},
			{.key = "z-8", .key_len = 3, .val = "z-index:8", .val_len = 9},
			{.key = "z-12", .key_len = 4, .val = "z-index:12", .val_len = 10},
			{.key = "z-16", .key_len = 4, .val = "z-index:16", .val_len = 10},
	};

	for (uint8_t index = 0; index < sizeof(layouts) / sizeof(keymap_t); index++) {
		if (cls->len == layouts[index].key_len && memcmp(cls->ptr, layouts[index].key, layouts[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &layouts[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

void table(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len) {
	const keymap_t tables[] = {
			{.key = "border-collapse", .key_len = 15, .val = "border-collapse:collapse", .val_len = 24},
			{.key = "border-separate", .key_len = 15, .val = "border-collapse:separate", .val_len = 24},
			{.key = "table-auto", .key_len = 10, .val = "table-layout:auto", .val_len = 17},
			{.key = "table-fixed", .key_len = 11, .val = "table-layout:fixed", .val_len = 18},
	};

	for (uint8_t index = 0; index < sizeof(tables) / sizeof(keymap_t); index++) {
		if (cls->len == tables[index].key_len && memcmp(cls->ptr, tables[index].key, tables[index].key_len) == 0) {
			cls->known = stamp(NULL, cls, &tables[index], NULL, NULL, buffer, buffer_len);
		}
	}
}

#pragma once

#include "hydrate.h"

typedef struct keymap_t {
	const char *key;
	const uint8_t key_len;
	const char *val;
	const uint8_t val_len;
} keymap_t;

void common(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);
void position(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);
void display(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint);
void spacing(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint);
void sizing(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint);
void border(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint);
void overflow(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint);
void flex(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint);
void text(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);
void font(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);
void color(class_t *pfx, class_t *cls, char (*buffer)[8192], uint16_t *buffer_len, breakpoint_t *breakpoint);
void opacity(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);
void cursor(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);
void layout(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);
void table(class_t *cls, char (*buffer)[8192], uint16_t *buffer_len);

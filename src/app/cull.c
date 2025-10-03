#include "cull.h"
#include "file.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

uint8_t cull_flight(file_t *file) {
	size_t read_index = 0;
	size_t write_index = 0;

	bool cut = false;
	cull_t culls[27] = {
			{.start = "\t\t\t.right-0 {", .end = "}\n"},
			{.start = "\t\t\t.m-2 {", .end = "}\n"},
			{.start = "\t\t\t.bg-amber-200 {", .end = "}\n"},
			{.start = "\t\t\t.bg-green-200 {", .end = "}\n"},
			{.start = "\t\t\t.active\\:scale-95:active {", .end = "}\n"},
			{.start = "\t\t\t.z-10 {", .end = "}\n"},
			{.start = "\t\t\t\t.md\\:m-4 {", .end = "}\n"},
			{.start = "\t\t\t\t.dark\\:bg-amber-800 {", .end = "}\n"},
			{.start = "\t\t\t\t.dark\\:bg-green-800 {", .end = "}\n"},
			{.start = "\t\t\t<div id=\"notifications\"", .end = "</div>\n"},
			{.start = "\t\t\t\t<a href=\"/settings\"", .end = "</a>\n"},
			{.start = "\t\t\t\t<button id=\"upload\"", .end = "</button>\n"},
			{.start = "\t\tconst upload", .end = ";\n"},
			{.start = "\t\tconst readFile", .end = "\n\t\t};\n"},
			{.start = "\t\tconst hashFile", .end = "\n\t\t};\n"},
			{.start = "\t\tconst postFlight", .end = "\n\t\t};\n"},
			{.start = "\t\tconst uploadFlights", .end = "\n\t\t};\n"},
			{.start = "\t\tconst parseIgcFile", .end = "\n\t\t};\n"},
			{.start = "\t\tconst radians", .end = "\n\t\t};\n"},
			{.start = "\t\tconst heading", .end = "\n\t\t};\n"},
			{.start = "\t\tconst distance", .end = "\n\t\t};\n"},
			{.start = "\t\tconst parseFlight", .end = "\n\t\t};\n"},
			{.start = "\t\tconst notifications", .end = ";\n"},
			{.start = "\t\tconst kind", .end = "\n\t\t};\n"},
			{.start = "\t\tconst color", .end = "\n\t\t};\n"},
			{.start = "\t\tconst duration", .end = "\n\t\t};\n"},
			{.start = "\t\tconst notification", .end = "\n\t\t};\n"},
	};
	uint8_t cull_index = 0;

	for (uint8_t index = 0; index < sizeof(culls) / sizeof(cull_t); index++) {
		cull_t *cull = &culls[index];
		cull->start_ind = 0;
		cull->start_len = (uint8_t)strlen(cull->start);
		cull->end_ind = 0;
		cull->end_len = (uint8_t)strlen(cull->end);
	}

	while (read_index < file->len) {
		char *byte = &file->ptr[read_index];

	top:
		if (cull_index < sizeof(culls) / sizeof(cull_t)) {
			cull_t *cull = &culls[cull_index];
			if (cut == false && cull->start_ind == cull->start_len) {
				cut = true;
				write_index -= cull->start_len;
			} else if (*byte == cull->start[cull->start_ind]) {
				cull->start_ind++;
			} else {
				cull->start_ind = 0;
			}

			if (cut == true && cull->end_ind == cull->end_len) {
				cut = false;
				cull_index++;
				goto top;
			} else if (*byte == cull->end[cull->end_ind]) {
				cull->end_ind++;
			} else {
				cull->end_ind = 0;
			}
		}

		file->ptr[write_index] = *byte;
		if (cut == false) {
			write_index++;
		}
		read_index++;
	}

	file->len = write_index;
	return sizeof(culls) / sizeof(cull_t) - cull_index;
}

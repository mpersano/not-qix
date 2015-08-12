#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include <vector>
#include <string>
#include <algorithm>

#include "font.h"
#include "sprite_packer.h"
#include "panic.h"

static void
usage(const char *argv0)
{
	fprintf(stderr, "usage: %s <options> font sheet_name ranges...\n", argv0);
	exit(EXIT_FAILURE);
}

static int
parse_int(const char *str)
{
	return *str == 'x' || *str == 'X' ? strtol(str + 1, 0, 16) : strtol(str, 0, 10);
}

int
main(int argc, char *argv[])
{
	int border = 2;
	int font_size = 16;
	int sheet_width = 256;
	int sheet_height = 256;
	int outline_radius = 2;
	bool multi = true;

	int c;

	while ((c = getopt(argc, argv, "b:s:w:h:g:1")) != EOF) {
		switch (c) {
			case 'b':
				border = atoi(optarg);
				break;

			case 's':
				font_size = atoi(optarg);
				break;

			case 'w':
				sheet_width = atoi(optarg);
				break;

			case 'h':
				sheet_height = atoi(optarg);
				break;
			
			case 'g':
				outline_radius = atoi(optarg);
				break;
			
			case '1':
				multi = false;
				break;
				
		}
	}

	if (argc - optind < 3)
		usage(*argv);

	const char *font_name = argv[optind];
	const char *sheet_name = argv[optind + 1];

	std::vector<sprite_base *> sprites;

	font f(font_name);
	f.set_char_size(font_size);

	for (int i = optind + 2; i < argc; i++) {
		char *range = argv[i];
		char *dash = strchr(range, '-');

		if (dash) {
			*dash = '\0';

			for (int j = parse_int(range); j <= parse_int(dash + 1); j++)
				sprites.push_back(f.render_glyph(j, outline_radius));
		} else {
			sprites.push_back(f.render_glyph(parse_int(range), outline_radius));
		}
	}

	// pack_sprites(sprites, pixmap::GRAY_ALPHA, border, sheet_name, sheet_width, sheet_height);
	sprite_packer *packer = sprite_packer::make(multi);
	packer->set_border(border);
	packer->set_sheet_size(sheet_width, sheet_height);
	packer->pack(sprites, sheet_name, pixmap::GRAY_ALPHA);
}

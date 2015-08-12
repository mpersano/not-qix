#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include <vector>

#include "pixmap.h"
#include "sprite.h"
#include "sprite_packer.h"
#include "panic.h"

static std::vector<sprite_base *> sprites;

static void
load_sprites(const char *dir_name)
{
	DIR *dir;

	if ((dir = opendir(dir_name)) == NULL)
		panic("failed to open %s: %s\n", dir_name, strerror(errno));

	dirent *de;

	while ((de = readdir(dir)) != NULL) {
		const char *name = de->d_name;
		size_t len = strlen(name);

		if (len >= 4 && !strcmp(name + len - 4, ".png")) {
			char path[PATH_MAX];
			sprintf(path, "%s/%s", dir_name, name);

			pixmap *pm = pixmap::load(path);
			margins m = pm->trim();

			sprite *sp = new sprite(name, m, pm);
			fprintf(stderr, "%s: %ux%u\n", name, sp->width(), sp->height());
			sprites.push_back(sp);
		}
	}

	closedir(dir);
}

static void
usage(const char *argv0)
{
	fprintf(stderr, "usage: %s <options> sheet_name path\n", argv0);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int c;
	int border = 2;
	int sheet_width = 256;
	int sheet_height = 256;
	bool multi = true;

	while ((c = getopt(argc, argv, "b:w:h:1")) != EOF) {
		switch (c) {
			case 'b':
				border = atoi(optarg);
				break;

			case 'w':
				sheet_width = atoi(optarg);
				break;

			case 'h':
				sheet_height = atoi(optarg);
				break;

			case '1':
				multi = false;
				break;
		}
	}

	if (argc - optind != 2)
		usage(*argv);

	const char *sheet_name = argv[optind];
	const char *dir_name = argv[optind + 1];

	load_sprites(dir_name);

	sprite_packer *packer = sprite_packer::make(multi);
	packer->set_border(border);
	packer->set_sheet_size(sheet_width, sheet_height);
	packer->pack(sprites, sheet_name, pixmap::RGB_ALPHA);
}

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
#include <memory>
#include <algorithm>

#include "font.h"
#include "sprite_packer.h"
#include "panic.h"

namespace {

class flat_color_fn : public color_fn
{
public:
	flat_color_fn(const rgb<int>& color)
	: color_(color)
	{ }

	rgb<int> operator()(float t) const override
	{ return color_; }

private:
	rgb<int> color_;
};

// lame linear gradient

class gradient_color_fn : public color_fn
{
public:
	gradient_color_fn(const rgb<int>& from, const rgb<int>& to)
	: from_(from), to_(to)
	{ }

	rgb<int> operator()(float t) const override
	{ return from_ + (to_ - from_)*t; }

private:
	rgb<int> from_, to_;
};

// quadratic bezier gradient

class bezier_color_fn : public color_fn
{
public:
	bezier_color_fn(const rgb<int>& c0, const rgb<int>& c1, const rgb<int>& c2)
	: c0_(c0), c1_(c1), c2_(c2)
	{ }

	rgb<int> operator()(float u) const override
	{
		const float w0 = (1 - u)*(1 - u);
		const float w1 = 2*u*(1 - u);
		const float w2 = u*u;

		return c0_*w0 + c1_*w1 + c2_*w2;
	}

private:
	rgb<int> c0_, c1_, c2_;
};

void
usage(const char *argv0)
{
	fprintf(stderr, "usage: %s <options> font sheet_name ranges...\n", argv0);
	exit(EXIT_FAILURE);
}

int
parse_int(const char *str)
{
	return *str == 'x' || *str == 'X' ? strtol(str + 1, 0, 16) : strtol(str, 0, 10);
}

rgb<int>
parse_color(const char *str)
{
	rgb<int> c;
	sscanf(str, "%02x%02x%02x", &c.r, &c.g, &c.b);
	return c;
}

std::unique_ptr<color_fn>
parse_color_fn(char *str)
{
	if (char *p = strchr(str, '-')) {
		*p = '\0';

		if (char *q = strchr(p + 1, '-')) {
			*q = '\0';

			return std::unique_ptr<color_fn>
				{
					new bezier_color_fn {
						parse_color(str),
						parse_color(p + 1),
						parse_color(q + 1) }
				};
		} else {
			return std::unique_ptr<color_fn>
				{
					new gradient_color_fn {
						parse_color(str),
						parse_color(p + 1) }
				};
		}
	} else {
		return std::unique_ptr<color_fn> { new flat_color_fn { parse_color(str) } };
	}
}

} // (anonymous namespace)

int
main(int argc, char *argv[])
{
	int border = 2;
	int font_size = 16;
	int sheet_width = 256;
	int sheet_height = 256;
	int outline_radius = 2;
	std::string texture_path_base = ".";

	std::unique_ptr<color_fn> inner_color { new flat_color_fn { rgb<int> { 255, 255, 255 } } };
	std::unique_ptr<color_fn> outline_color { new flat_color_fn { rgb<int> { 0, 0, 0 } } };

	int c;

	while ((c = getopt(argc, argv, "b:s:w:h:g:t:i:o:")) != EOF) {
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

			case 't':
				texture_path_base = optarg;
				break;

			case 'i':
				inner_color = std::move(parse_color_fn(optarg));
				break;

			case 'o':
				outline_color = std::move(parse_color_fn(optarg));
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

		if (char *dash = strchr(range, '-')) {
			*dash = '\0';

			for (int j = parse_int(range); j <= parse_int(dash + 1); j++)
				sprites.push_back(f.render_glyph(j, outline_radius, *inner_color.get(), *outline_color.get()));
		} else {
			sprites.push_back(f.render_glyph(parse_int(range), outline_radius, *inner_color.get(), *outline_color.get()));
		}
	}

	pack_sprites(sprites,
			sheet_name,
			sheet_width, sheet_height,
			border,
			pixmap::RGB_ALPHA,
			texture_path_base.c_str());
}

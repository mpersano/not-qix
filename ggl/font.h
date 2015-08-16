#pragma once

#include <cstddef>
#include <wchar.h>

#include <string>

#include <ggl/gl.h>

namespace ggl {

class texture;

struct glyph
{
	glyph(const ggl::texture *tex, int left, int top, int advance_x, int u, int v, int width, int height);

	const ggl::texture *tex;
	int width, height;
	int left, top, advance_x;
	float u0, u1, v0, v1;
};

class font
{
public:
	font(const std::string& path_base);
	~font();

	const glyph *find_glyph(wchar_t ch) const
	{ return glyph_map_[ch]; }

	unsigned get_string_width(const std::basic_string<wchar_t>& str) const;

	void render(const std::basic_string<wchar_t>& str) const;

	const texture *tex;

private:
	glyph *glyph_map_[1<<16];
};

}

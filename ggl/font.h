#pragma once

#include <cstddef>
#include <wchar.h>

#include <string>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/vertex_array.h>

namespace ggl {

class texture;

struct glyph
{
	glyph(const texture *tex, int u, int v, int width, int height, int left, int top, int advance_x);

	int left, top, advance_x;
	sprite spr;
};

class font
{
public:
	font(const std::string& path);
	~font();

	const glyph *get_glyph(wchar_t ch) const
	{ return glyph_map_[ch]; }

	unsigned get_string_width(const std::basic_string<wchar_t>& str) const;

	void render(const std::basic_string<wchar_t>& str, ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2>& va) const;

private:
	glyph *glyph_map_[1<<16];
};

}

#pragma once

#include <cstddef>
#include <wchar.h>

#include <string>

#include <ggl/gl.h>
#include <ggl/vec2.h>

namespace ggl {

class texture;

struct glyph_info
{
	int width, height;
	int left, top;
	int advance_x;
	vec2f texuv[2]; // texture coordinates
};

class font
{
public:
	font(const std::string& path_base);
	~font();

	const glyph_info *find_glyph(wchar_t ch) const
	{ return glyph_info_map_[ch]; }

	unsigned get_string_width(const std::basic_string<wchar_t>& str) const;

	const texture *get_texture() const
	{ return texture_; }

	void render(const std::basic_string<wchar_t>& str) const;

private:
	glyph_info *glyph_info_map_[1<<16];
	const texture *texture_;
};

}

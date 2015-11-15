#pragma once

#include <cstddef>
#include <wchar.h>

#include <string>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/align.h>
#include <ggl/vec2.h>
#include <ggl/rgba.h>

namespace ggl {

class texture;
class sprite_batch;

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

	void draw(sprite_batch& sb, float depth, const std::wstring& str) const;
	void draw(sprite_batch& sb, float depth, const std::wstring& str, const vec2f& pos) const;
	void draw(sprite_batch& sb, float depth, const std::wstring& str, const vec2f& pos, vert_align va, horiz_align ha) const;

private:
	glyph *glyph_map_[1<<16];
};

}

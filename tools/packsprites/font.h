#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "rgb.h"
#include "sprite_base.h"

struct glyph : sprite_base
{
	glyph(wchar_t code, int left, int top, int advance_x, pixmap *pm);

	void serialize(TiXmlElement *el) const override;

	wchar_t code_;
	int left_, top_, advance_x_;
};

class color_fn
{
public:
	virtual ~color_fn() = default;

	virtual rgb<int> operator()(float t) const = 0;
};

class font
{
public:
	font(const char *path);
	virtual ~font();

	void set_char_size(int size);

	glyph *render_glyph(
			const wchar_t code,
			int outline_radius,
			const color_fn& inner_color,
			const color_fn& outline_color,
			int shadow_dx, int shadow_dy,
			float shadow_opacity);

private:
	FT_Face face_;
};

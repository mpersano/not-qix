#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "sprite_base.h"

struct glyph : sprite_base
{
	glyph(wchar_t code, int left, int top, int advance_x, pixmap *pm);

	void serialize(FILE *out, const rect& rc, int border) const override;

	wchar_t code_;
	int left_, top_, advance_x_;
};

class font
{
public:
	font(const char *path);
	virtual ~font();

	void set_char_size(int size);
	glyph *render_glyph(const wchar_t code, int outline_radius);

private:
	FT_Face face_;
};

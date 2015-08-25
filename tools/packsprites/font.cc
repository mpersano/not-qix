#include <algorithm>
#include <cmath>

#include "rect.h"
#include "rgb.h"
#include "panic.h"
#include "font.h"

glyph::glyph(wchar_t code, int left, int top, int advance_x, pixmap *pm)
: sprite_base(pm)
, code_(code)
, left_(left)
, top_(top)
, advance_x_(advance_x)
{ }

void
glyph::serialize(FILE *out, const rect& rc, int border) const
{
	fprintf(out, "    <glyph x=\"%d\" y=\"%d\" w=\"%d\" h=\"%d\" code=\"%d\" left=\"%d\" top=\"%d\" advancex=\"%d\" />\n",
		rc.left_ + border, rc.top_ + border,
		width(), height(),
		code_, left_, top_, advance_x_);
}

class ft_library
{
public:
	static FT_Library get_instance();

private:
	ft_library();
	~ft_library();

	FT_Library library_;
};

ft_library::ft_library()
{
	if (FT_Init_FreeType(&library_) != 0)
		panic("FT_Init_FreeType");
}

ft_library::~ft_library()
{
	FT_Done_FreeType(library_);
}

FT_Library
ft_library::get_instance()
{
	static ft_library instance;
	return instance.library_;
}

font::font(const char *path)
{
	if (FT_New_Face(ft_library::get_instance(), path, 0, &face_) != 0)
		panic("FT_New_Face");
}

font::~font()
{
	FT_Done_Face(face_);
}

void
font::set_char_size(int size)
{
	if (FT_Set_Char_Size(face_, size << 6, 0, 100, 0) != 0)
		panic("FT_Set_Char_Size");
}

glyph *
font::render_glyph(wchar_t code, int outline_radius, const color_fn& inner_color, const color_fn& outline_color)
{
	if ((FT_Load_Char(face_, code, FT_LOAD_RENDER)) != 0)
		panic("FT_Load_Char");

	FT_GlyphSlot slot = face_->glyph;

	FT_Bitmap *bitmap = &slot->bitmap;

	const int src_height = bitmap->rows;
	const int src_width = bitmap->pitch;

	const int dest_height = src_height + 2*outline_radius;
	const int dest_width = src_width + 2*outline_radius;

	std::vector<float> gray(dest_width*dest_height), alpha(dest_width*dest_height);

	// copy grayscale channel
	// XXX: don't really need this step

	{
	const uint8_t *p = bitmap->buffer;
	float *q = &gray[outline_radius*dest_width + outline_radius];

	for (int i = 0; i < src_height; i++) {
		for (int j = 0; j < src_width; j++) {
			*q++ = static_cast<float>(*p++)/255.;
		}

		q += 2*outline_radius;
	}
	}

	// create outline with dilation morphological filter

	{
	float kernel[2*outline_radius + 1][2*outline_radius + 1];

	for (int i = 0; i < 2*outline_radius + 1; i++) {
		for (int j = 0; j < 2*outline_radius + 1; j++) {
			const int dr = i - outline_radius;
			const int dc = j - outline_radius;

			const float l = sqrtf(dr*dr + dc*dc);

			if (l <= outline_radius)
				kernel[i][j] = 1.;
			else if (l < outline_radius + 1.)
				kernel[i][j] = 1. - (l - outline_radius);
			else
				kernel[i][j] = 0.;
		}
	}

	float *p = &alpha[0];

	for (int i = 0; i < dest_height; i++) {
		for (int j = 0; j < dest_width; j++) {
			float v = 0;

			for (int dr = -outline_radius; dr <= outline_radius; dr++) {
				for (int dc = -outline_radius; dc <= outline_radius; dc++) {
					int r = i + dr;
					int c = j + dc;

					if (r >= 0 && r < dest_height  && c >= 0 && c < dest_width) {
						const float w = kernel[dr + outline_radius][dc + outline_radius];
						v = std::max(v, w*gray[r*dest_width + c]);
					}
				}
			}

			*p++ = v;
		}
	}
	}

	pixmap *pm = new pixmap(dest_width, dest_height, pixmap::RGB_ALPHA);

	// initialize pixmap, add some color

	{
	uint32_t *p = reinterpret_cast<uint32_t *>(pm->get_bits());

	for (int i = 0; i < dest_height; i++) {
		const float t = static_cast<float>(i)/dest_height; // XXX: erm not really, should sub outline radius for inner color

		rgb<int> fg_color = inner_color(t);
		rgb<int> bg_color = outline_color(t);

		for (int j = 0; j < dest_width; j++) {
			rgb<int> c = bg_color + (fg_color - bg_color)*gray[i*dest_width + j];
			int a = static_cast<int>(255*alpha[i*dest_width + j]);
			*p++ = c.r + (c.g << 8) + (c.b << 16) + (a << 24);
		}
	}
	}

	const FT_Glyph_Metrics *metrics = &slot->metrics;
	const int left = metrics->horiBearingX >> 6;
	const int top = metrics->horiBearingY >> 6;
	const int advance_x = metrics->horiAdvance >> 6;

	return new glyph(code, left, top, advance_x, pm);
}

#include <algorithm>
#include <cmath>

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
glyph::serialize(file_writer& fw) const
{
	fw.write_uint16(code_);
	fw.write_uint8(left_);
	fw.write_uint8(top_);
	fw.write_uint8(advance_x_);
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
font::render_glyph(wchar_t code, int outline_radius)
{
	if ((FT_Load_Char(face_, code, FT_LOAD_RENDER)) != 0)
		panic("FT_Load_Char");

	FT_GlyphSlot slot = face_->glyph;

	FT_Bitmap *bitmap = &slot->bitmap;

	const int src_height = bitmap->rows;
	const int src_width = bitmap->pitch;

	const int dest_height = src_height + 2*outline_radius;
	const int dest_width = src_width + 2*outline_radius;

	pixmap *pm = new pixmap(dest_width, dest_height, pixmap::GRAY_ALPHA);

	// copy grayscale channel

	const uint8_t *q = bitmap->buffer;

	for (int i = 0; i < src_height; i++) {
		uint16_t *p = reinterpret_cast<uint16_t *>(pm->get_bits()) + (outline_radius + i)*dest_width + outline_radius;

		for (int j = 0; j < src_width; j++) {
			uint8_t v = *q++;
			*p++ = v;
		}
	}

	// apply dilation morphological filter to alpha channel

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

	uint16_t *p = reinterpret_cast<uint16_t *>(pm->get_bits());

	for (int i = 0; i < dest_height; i++) {
		for (int j = 0; j < dest_width; j++) {
			uint8_t v = 0;

			for (int dr = -outline_radius; dr <= outline_radius; dr++) {
				for (int dc = -outline_radius; dc <= outline_radius; dc++) {
					int r = i + dr - outline_radius;
					int c = j + dc - outline_radius;

					if (r >= 0 && r < src_height && c >= 0 && c < src_width) {
						const float w = kernel[dr + outline_radius][dc + outline_radius];
						v = std::max(v, static_cast<uint8_t>(w*bitmap->buffer[r*src_width + c]));
					}
				}
			}

			*p++ |= static_cast<uint16_t >(v) << 8;
		}
	}

	const FT_Glyph_Metrics *metrics = &slot->metrics;
	const int left = metrics->horiBearingX >> 6;
	const int top = metrics->horiBearingY >> 6;
	const int advance_x = metrics->horiAdvance >> 6;

	return new glyph(code, left, top, advance_x, pm);
}

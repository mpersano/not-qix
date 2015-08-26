#include <algorithm>
#include <cmath>

#include <tinyxml.h>

#include "rect.h"
#include "rgb.h"
#include "panic.h"
#include "font.h"

namespace {

template <typename T>
struct image
{
	image(int width, int height)
	: width { width }
	, height { height }
	, data(width*height)
	{ }

	image(int width, int height, uint8_t *initial_data)
	: width { width }
	, height { height }
	, data(width*height)
	{
		std::transform(
			initial_data,
			initial_data + width*height,
			std::begin(data),
			[](uint8_t v) { return static_cast<float>(v)/255.f; });
	}

	void copy(const image<T>& other, int dr, int dc);

	image&
	operator*=(float s)
	{
		std::transform(
			std::begin(data),
			std::end(data),
			std::begin(data),
			[=](float v) { return s*v; });

		return *this;
	}

	int width, height;
	std::vector<T> data;
};

template <typename T>
void
image<T>::copy(const image<T>& other, int dr, int dc)
{
	int r0 = std::max(dr, 0);
	int r1 = std::min(dr + other.height, height);

	int c0 = std::max(dc, 0);
	int c1 = std::min(dc + other.width, width);

	for (int r = r0; r < r1; r++)
		for (int c = c0; c < c1; c++)
			data[r*width + c] = other.data[(r - dr)*other.width + (c - dc)];
}

template <typename T>
image<T>
dilate(const image<T>& im, int radius)
{
	// would be faster with two passes, but I don't care

	float kernel[2*radius + 1][2*radius + 1];

	for (int i = 0; i < 2*radius + 1; i++) {
		for (int j = 0; j < 2*radius + 1; j++) {
			const int dr = i - radius;
			const int dc = j - radius;

			const float l = sqrtf(dr*dr + dc*dc);

			if (l <= radius)
				kernel[i][j] = 1.;
			else if (l < radius + 1.)
				kernel[i][j] = 1. - (l - radius);
			else
				kernel[i][j] = 0.;
		}
	}

	image<T> rv(im.width, im.height);

	auto dest = std::begin(rv.data);

	for (int i = 0; i < im.height; i++) {
		for (int j = 0; j < im.width; j++) {
			float v = 0;

			for (int dr = -radius; dr <= radius; dr++) {
				for (int dc = -radius; dc <= radius; dc++) {
					int r = i + dr;
					int c = j + dc;

					if (r >= 0 && r < im.height  && c >= 0 && c < im.width) {
						const float w = kernel[dr + radius][dc + radius];
						v = std::max(v, w*im.data[r*im.width + c]);
					}
				}
			}

			*dest++ = v;
		}
	}

	return rv;
}

} // (anonymous namespace)

glyph::glyph(wchar_t code, int left, int top, int advance_x, pixmap *pm)
: sprite_base(pm)
, code_(code)
, left_(left)
, top_(top)
, advance_x_(advance_x)
{ }

void
glyph::serialize(TiXmlElement *el) const
{
	el->SetAttribute("code", code_);
	el->SetAttribute("left", left_);
	el->SetAttribute("top", top_);
	el->SetAttribute("advancex", advance_x_);
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
font::render_glyph(
		wchar_t code,
		int outline_radius,
		const color_fn& inner_color,
		const color_fn& outline_color,
		int shadow_dx, int shadow_dy,
		float shadow_opacity)
{
	if ((FT_Load_Char(face_, code, FT_LOAD_RENDER)) != 0)
		panic("FT_Load_Char");

	FT_GlyphSlot slot = face_->glyph;

	FT_Bitmap *bitmap = &slot->bitmap;

	const int src_height = bitmap->rows;
	const int src_width = bitmap->pitch;

	int dest_height = src_height + 2*outline_radius;
	int dest_width = src_width + 2*outline_radius;

	int offset_x = outline_radius;
	int offset_y = outline_radius;

	if (shadow_dx < 0) {
		dest_width += -shadow_dx;
		offset_x += -shadow_dx;
	} else if (shadow_dx > 0) {
		dest_width += shadow_dx;
	}

	if (shadow_dy < 0) {
		dest_height += -shadow_dy;
		offset_y += -shadow_dy;
	} else if (shadow_dy > 0) {
		dest_height += shadow_dy;
	}

	// copy grayscale channel

	image<float> lum(dest_width, dest_height);
	lum.copy(image<float>(src_width, src_height, bitmap->buffer), offset_y, offset_x);

	// create outline with dilation morphological filter

	image<float> alpha = dilate(lum, outline_radius);

	// drop shadow

	image<float> shadow(dest_width, dest_height);
	shadow.copy(alpha, shadow_dy, shadow_dx);
	shadow *= shadow_opacity;

	// initialize pixmap, adding some color

	pixmap *pm = new pixmap(dest_width, dest_height, pixmap::RGB_ALPHA);

	{
	auto src_lum = std::begin(lum.data);
	auto src_alpha = std::begin(alpha.data);
	auto src_shadow = std::begin(shadow.data);

	auto *dest = reinterpret_cast<uint32_t *>(pm->get_bits());

	for (int i = 0; i < dest_height; i++) {
		const float t = static_cast<float>(i)/dest_height; // XXX: should sub outline radius for inner color

		const rgb<int> c0 = inner_color(t);
		const rgb<int> c1 = outline_color(t);

		for (int j = 0; j < dest_width; j++) {
#if 1
			rgb<int> fg_color = c1 + (c0 - c1)*(*src_lum++);

			float s = *src_shadow++;

			rgb<int> bg_color { 0, 0, 0 };

			float o = *src_alpha++;
			rgb<int> color = bg_color + (fg_color - bg_color)*o;

			int a = static_cast<int>(255*(s + (1. - s)*o));

			*dest++ = color.r + (color.g << 8) + (color.b << 16) + (a << 24);
#else
			rgb<int> color = c1 + (c0 - c1)*(*src_lum++);

			int a = static_cast<int>(255*(*src_alpha++));

			*dest++ = color.r + (color.g << 8) + (color.b << 16) + (a << 24);
#endif
		}
	}
	}

	const FT_Glyph_Metrics *metrics = &slot->metrics;
	const int left = metrics->horiBearingX >> 6;
	const int top = metrics->horiBearingY >> 6;
	const int advance_x = metrics->horiAdvance >> 6;

	return new glyph(code, left, top, advance_x, pm);
}

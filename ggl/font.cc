#include <cstring>
#include <cerrno>
#include <algorithm>
#include <stdarg.h>

#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/texture.h>
#include <ggl/resources.h>
#include <ggl/vertex_array.h>
#include <ggl/font.h>

namespace ggl {

glyph::glyph(const texture *tex, int left, int top, int advance_x, int u, int v, int width, int height)
: tex { tex }
, width { width }
, height { height }
, left { left }
, top { top }
, advance_x { advance_x }
{
	const int tex_width = tex->width;
	const int tex_height = tex->height;

	const float du = static_cast<float>(width)/tex_width;
	const float dv = static_cast<float>(height)/tex_height;

	u0 = static_cast<float>(u)/tex_width;
	v0 = static_cast<float>(tex_height - v)/tex_height;

	u1 = u0 + du;
	v1 = v0 - dv;
}

font::font(const std::string& path_base)
{
	std::fill(std::begin(glyph_map_), std::end(glyph_map_), nullptr);

	auto font_asset = g_core->get_asset(path_base + ".spr");

	unsigned num_glyphs = font_asset->read_uint16();

	tex = res::get_texture(path_base + ".png");

	for (unsigned i = 0; i < num_glyphs; i++) {
		wchar_t code = font_asset->read_uint16();

		int left = static_cast<int8_t>(font_asset->read_uint8());
		int top = static_cast<int8_t>(font_asset->read_uint8());
		int advance_x = static_cast<int8_t>(font_asset->read_uint8());

		const int u = font_asset->read_uint16();
		const int v = font_asset->read_uint16();
		const int width = font_asset->read_uint16();
		const int height = font_asset->read_uint16();

		glyph_map_[code] = new glyph { tex, left, top, advance_x, u, v, width, height };
	}
}

font::~font()
{ }

unsigned
font::get_string_width(const std::basic_string<wchar_t>& str) const
{
	return std::accumulate(
		std::begin(str),
		std::end(str),
		0u,
		[this](unsigned width, wchar_t ch)
			{
				return width + find_glyph(ch)->advance_x;
			});
}

void
font::render(const std::basic_string<wchar_t>& str) const
{
	vertex_array_texcoord<GLshort, 2, GLfloat, 2> va;
	va.reserve(6*str.size());

	const int y = 0;
	int x = 0;

	for (wchar_t ch : str) {
		auto *g = find_glyph(ch);

		short x0 = x + g->left;
		short x1 = x0 + g->width;
		short y0 = y + g->top;
		short y1 = y0 - g->height;

		const float u0 = g->u0;
		const float u1 = g->u1;
		const float v0 = g->v0;
		const float v1 = g->v1;

		va.push_back({ x0, y0, u0, v0 });
		va.push_back({ x1, y0, u1, v0 });
		va.push_back({ x1, y1, u1, v1 });

		va.push_back({ x1, y1, u1, v1 });
		va.push_back({ x0, y1, u0, v1 });
		va.push_back({ x0, y0, u0, v0 });

		x += g->advance_x;
	}

	glEnable(GL_TEXTURE_2D);
	tex->bind();

	va.draw(GL_TRIANGLES);
}

}

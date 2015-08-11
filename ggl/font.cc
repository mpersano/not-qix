#include <cstring>
#include <cerrno>
#include <stdarg.h>

#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/xwchar.h>
#include <ggl/texture.h>
#include <ggl/resources.h>
#include <ggl/vertex_array.h>
#include <ggl/font.h>

namespace ggl {

font::font(const std::string& path_base)
{
	std::fill(std::begin(glyph_info_map_), std::end(glyph_info_map_), nullptr);

	auto font_asset = g_core->get_asset(path_base + ".spr");

	unsigned num_glyphs = font_asset->read_uint16();

	texture_ = res::get_texture(path_base + ".png");

	for (unsigned i = 0; i < num_glyphs; i++) {
		wchar_t code = font_asset->read_uint16();

		int left = static_cast<int8_t>(font_asset->read_uint8());
		int top = static_cast<int8_t>(font_asset->read_uint8());
		int advance_x = static_cast<int8_t>(font_asset->read_uint8());

		const int u = font_asset->read_uint16();
		const int v = font_asset->read_uint16();
		const int w = font_asset->read_uint16();
		const int h = font_asset->read_uint16();

		glyph_info *g = new glyph_info;
		g->width = w;
		g->height = h;
		g->left = left;
		g->top = top;
		g->advance_x = advance_x;

		const int texture_width = texture_->width;
		const int texture_height = texture_->height;

		const float u0 = static_cast<float>(u)/texture_width;
		const float v0 = static_cast<float>(texture_height - 1 - v)/texture_height;

		const float du = static_cast<float>(w)/texture_width;
		const float dv = static_cast<float>(h)/texture_height;

		g->texuv[0] = { u0, v0 };
		g->texuv[1] = { u0 + du, v0 };
		g->texuv[2] = { u0 + du, v0 - dv };
		g->texuv[3] = { u0, v0 - dv };

		glyph_info_map_[code] = g;
	}
}

font::~font()
{ }

unsigned
font::get_string_width(const wchar_t *str) const
{
	return get_string_width(str, xwcslen(str));
}

unsigned
font::get_string_width(const wchar_t *str, size_t len) const
{
	unsigned width = 0;

	for (const wchar_t *p = str; p != &str[len]; p++)
		width += find_glyph(*p)->advance_x;

	return width;
}

void
font::render(const wchar_t *str) const
{
	vertex_array_texcoord<GLshort, 2, GLfloat, 2> va;
	va.reserve(6*xwcslen(str));

	const int y = 0;
	int x = 0;

	for (const wchar_t *p = str; *p; p++) {
		auto *g = find_glyph(*p);

		short x0 = x + g->left;
		short x1 = x0 + g->width;
		short y0 = y + g->top;
		short y1 = y0 - g->height;

		auto& t0 = g->texuv[0];
		auto& t1 = g->texuv[1];
		auto& t2 = g->texuv[2];
		auto& t3 = g->texuv[3];

		va.push_back({ x0, y0, t0.x, t0.y });
		va.push_back({ x1, y0, t1.x, t1.y });
		va.push_back({ x1, y1, t2.x, t2.y });

		va.push_back({ x1, y1, t2.x, t2.y });
		va.push_back({ x0, y1, t3.x, t3.y });
		va.push_back({ x0, y0, t0.x, t0.y });

		x += g->advance_x;
	}

	glEnable(GL_TEXTURE_2D);
	texture_->bind();

	va.draw(GL_TRIANGLES);
}

}

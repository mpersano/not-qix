#include <cstring>
#include <cerrno>
#include <algorithm>
#include <stdarg.h>

#include <tinyxml.h>

#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/texture.h>
#include <ggl/resources.h>
#include <ggl/util.h>
#include <ggl/font.h>

namespace ggl {

glyph::glyph(const texture *tex, int u, int v, int width, int height, int left, int top, int advance_x)
: spr { tex, u, v, width, height }
, left { left }
, top { top }
, advance_x { advance_x }
{ }

font::font(const std::string& path)
{
	auto asset = g_core->get_asset(path);

	std::vector<char> xml(asset->size());
	asset->read(&xml[0], asset->size());

	TiXmlDocument doc;
	doc.Parse(&xml[0]);

	TiXmlElement *root_el = doc.RootElement();

	// texture

	if (TiXmlElement *texture_el = root_el->FirstChildElement("texture")) {
		tex = res::get_texture(texture_el->Attribute("path"));
	}

	// glyphs

	std::fill(std::begin(glyph_map_), std::end(glyph_map_), nullptr);

	if (TiXmlElement *glyphs_el = root_el->FirstChildElement("sprites")) {
		for (TiXmlNode *node = glyphs_el->FirstChild(); node; node = node->NextSibling()) {
			TiXmlElement *el = node->ToElement();
			if (!el)
				continue;

			// XXX: error checking

			wchar_t code = atoi(el->Attribute("code"));

			int u = atoi(el->Attribute("x"));
			int v = atoi(el->Attribute("y"));
			int width = atoi(el->Attribute("w"));
			int height = atoi(el->Attribute("h"));

			int left = atoi(el->Attribute("left"));
			int top = atoi(el->Attribute("top"));
			int advance_x = atoi(el->Attribute("advancex"));

			glyph_map_[code] = new glyph { tex, u, v, width, height, left, top, advance_x };
		}
	}
}

font::~font()
{
	for (auto p : glyph_map_) {
		if (p)
			delete p;
	}
}

unsigned
font::get_string_width(const std::basic_string<wchar_t>& str) const
{
	return std::accumulate(
		std::begin(str),
		std::end(str),
		0u,
		[this](unsigned width, wchar_t ch)
			{
				return width + glyph_map_[ch]->advance_x;
			});
}

void
font::render(const std::basic_string<wchar_t>& str) const
{
	enable_alpha_blend _;
	enable_texture __;

	vertex_array_texcoord<GLshort, 2, GLfloat, 2> va;
	render(str, va);

	tex->bind();

	va.draw(GL_TRIANGLES);
}

void
font::render(const std::basic_string<wchar_t>& str, ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2>& va) const
{
	va.reserve(6*str.size());

	int x = 0;

	for (wchar_t ch : str) {
		auto g = glyph_map_[ch];

		short x0 = x + g->left;
		short x1 = x0 + g->spr.width;
		short y0 = g->top;
		short y1 = y0 - g->spr.height;

		const float u0 = g->spr.u0;
		const float u1 = g->spr.u1;
		const float v0 = g->spr.v0;
		const float v1 = g->spr.v1;

		va.push_back({ x0, y0, u0, v0 });
		va.push_back({ x1, y0, u1, v0 });
		va.push_back({ x1, y1, u1, v1 });

		va.push_back({ x1, y1, u1, v1 });
		va.push_back({ x0, y1, u0, v1 });
		va.push_back({ x0, y0, u0, v0 });

		x += g->advance_x;
	}
}

}

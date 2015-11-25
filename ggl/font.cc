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
#include <ggl/render.h>
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

	auto root_el = doc.RootElement();

	// texture

	std::vector<const texture *> textures;

	if (auto textures_el  = root_el->FirstChildElement("textures")) {
		for (auto node = textures_el->FirstChild(); node; node = node->NextSibling()) {
			if (auto el = node->ToElement())
				textures.push_back(res::get_texture(el->Attribute("path")));
		}
	}
	// glyphs

	std::fill(std::begin(glyph_map_), std::end(glyph_map_), nullptr);

	if (auto glyphs_el = root_el->FirstChildElement("sprites")) {
		for (auto node = glyphs_el->FirstChild(); node; node = node->NextSibling()) {
			auto el = node->ToElement();
			if (!el)
				continue;

			// XXX: error checking

			int tex = atoi(el->Attribute("tex"));

			int u = atoi(el->Attribute("x"));
			int v = atoi(el->Attribute("y"));
			int width = atoi(el->Attribute("w"));
			int height = atoi(el->Attribute("h"));

			wchar_t code = atoi(el->Attribute("code"));

			int left = atoi(el->Attribute("left"));
			int top = atoi(el->Attribute("top"));
			int advance_x = atoi(el->Attribute("advancex"));

			glyph_map_[code] = new glyph { textures[tex], u, v, width, height, left, top, advance_x };
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

void
font::draw(float depth, const std::wstring& str) const
{
	draw(depth, str, { 0.f, 0.f });
}

void
font::draw(float depth, const std::wstring& str, const vec2f& pos) const
{
	draw(depth, str, pos, vert_align::CENTER, horiz_align::CENTER);
}

void
font::draw(float depth, const std::wstring& str, const vec2f& pos, vert_align va, horiz_align ha) const
{
	assert(!str.empty());

	int x_min = glyph_map_[str.front()]->left;

	int x_max =
		std::accumulate(
			std::begin(str),
			std::end(str) - 1,
			x_min,
			[this](int s, wchar_t ch) { return s + glyph_map_[ch]->advance_x; }) +
		[this, &str] { auto g = glyph_map_[str.back()]; return g->left + g->spr.width; }(); // tee-hee

	int y_max =
		std::accumulate(
			std::begin(str),
			std::end(str),
			0,
			[this](int s, wchar_t ch) { return std::max(s, glyph_map_[ch]->top); });

	int y_min =
		std::accumulate(
			std::begin(str),
			std::end(str),
			0,
			[this](int s, wchar_t ch) { auto g = glyph_map_[ch]; return std::min(s, g->top - g->spr.height); });

	float x = pos.x;

	switch (ha) {
		case horiz_align::LEFT:
			x -= x_min;
			break;

		case horiz_align::CENTER:
			x -= .5f*(x_max + x_min);
			break;

		case horiz_align::RIGHT:
			x -= x_max;
			break;
	}

	float y = pos.y;

	switch (va) {
		case vert_align::BOTTOM:
			y -= y_min;
			break;

		case vert_align::CENTER:
			y -= .5f*(y_max + y_min);
			break;

		case vert_align::TOP:
			y -= y_max;
			break;
	}

	for (auto ch : str) {
		auto g = glyph_map_[ch];

		vec2f p0 { x + g->left, y + g->top };
		vec2f p1 = p0 + vec2f { g->spr.width, -g->spr.height };

		const auto& sp = g->spr;
		render::draw(sp.tex, { { sp.u0, sp.v0 }, { sp.u1, sp.v1 } }, bbox { p0, p1 }, depth );

		x += g->advance_x;
	}
}

}

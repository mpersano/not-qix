#include <cassert>
#include <algorithm>

#include <tinyxml.h>

#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/texture.h>
#include <ggl/resources.h>

#include "level.h"

std::vector<std::unique_ptr<level>> g_levels;

level::level(const std::string& fg_path, const std::string& bg_path, const std::string& mask_path)
: name { L"test" }
, fg_texture { ggl::res::get_texture(fg_path) }
, bg_texture { ggl::res::get_texture(bg_path) }
{
	ggl::image mask { mask_path };

	assert(mask.width%CELL_SIZE == 0);
	assert(mask.height%CELL_SIZE == 0);

	assert(fg_texture->orig_width == mask.width);
	assert(fg_texture->orig_height == mask.height);

	assert(bg_texture->orig_width == mask.width);
	assert(bg_texture->orig_height == mask.height);

	assert(mask.type == ggl::pixel_type::GRAY);

	const unsigned row_stride = mask.row_stride();
	const uint8_t *mask_pixels = &mask.data[0];

	grid_rows = mask.height/CELL_SIZE;
	grid_cols = mask.width/CELL_SIZE;

	silhouette.resize(grid_rows*grid_cols);

	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_cols; c++) {
			int s = 0;

			auto *p = &mask_pixels[r*CELL_SIZE*row_stride + c*CELL_SIZE];

			for (int i = 0; i < CELL_SIZE; i++) {
				s += std::accumulate(p, p + CELL_SIZE, 0, [](int s, uint8_t v) { return s + !!v; });
				p += row_stride;
			}

			silhouette[(grid_rows - r - 1)*grid_cols + c] = s;
		}
	}

	silhouette_pixels = std::accumulate(std::begin(silhouette), std::end(silhouette), 0u);
}

namespace {

const char *LEVELS_XML_PATH = "data/levels.xml";

std::unique_ptr<level>
level_from_xml_node(TiXmlNode *level_node)
{
	std::unique_ptr<level> rv;

	if (TiXmlElement *element = level_node->ToElement()) {
		std::string fg_path, bg_path, mask_path;

		for (TiXmlNode *node = element->FirstChild(); node; node = node->NextSibling()) {
			TiXmlElement *e = node->ToElement();
			if (!e)
				continue;

			const char *value = e->Value();

			if (strcmp(value, "foreground") == 0) {
				fg_path = e->Attribute("path");
			} else if (strcmp(value, "background") == 0) {
				bg_path = e->Attribute("path");
			} else if (strcmp(value, "mask") == 0) {
				mask_path = e->Attribute("path");
			}
		}

		rv.reset(new level { fg_path, bg_path, mask_path });
	}

	return rv;
}

};

void
init_levels()
{
	auto asset = ggl::g_core->get_asset(LEVELS_XML_PATH);

	std::vector<char> xml(asset->size());
	asset->read(&xml[0], asset->size());

	TiXmlDocument doc;
	doc.Parse(&xml[0]);

	if (doc.Error())
		panic("error parsing `%s': %s", LEVELS_XML_PATH, doc.ErrorDesc());

	if (TiXmlElement *levels = doc.RootElement()->FirstChildElement("levels")) {
		for (TiXmlNode *node = levels->FirstChild(); node; node = node->NextSibling())
			g_levels.push_back(level_from_xml_node(node));
	}
}

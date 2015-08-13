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

level::level(const std::string& background, const std::string& mask)
: background_texture { ggl::res::get_texture(background) }
, mask_texture { ggl::res::get_texture(mask) }
{
	assert(background_texture->orig_width == mask_texture->orig_width);
	assert(background_texture->orig_height == mask_texture->orig_height);
	assert(mask_texture->orig_width%CELL_SIZE == 0);
	assert(mask_texture->orig_height%CELL_SIZE == 0);

	const unsigned row_stride = mask_texture->row_stride();
	const unsigned pixel_size = mask_texture->pixel_size();
	const uint8_t *mask_pixels = &mask_texture->data[0];

	grid_rows = mask_texture->orig_height/CELL_SIZE;
	grid_cols = mask_texture->orig_width/CELL_SIZE;

	silhouette.resize(grid_rows*grid_cols);

	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_cols; c++) {
			int s = 0;

			auto *p = &mask_pixels[r*CELL_SIZE*row_stride + c*CELL_SIZE*pixel_size];

			for (int i = 0; i < CELL_SIZE; i++) {
				for (int j = 0; j < CELL_SIZE; j++) {
					s += *p != 0;
					p += pixel_size;
				}

				p += row_stride - CELL_SIZE*pixel_size;
			}

			silhouette[r*grid_cols + c] = s;
		}
	}

	silhouette_pixels = std::accumulate(std::begin(silhouette), std::end(silhouette), 0);
}

namespace {

const char *LEVELS_XML_PATH = "data/levels.xml";

std::unique_ptr<level>
level_from_xml_node(TiXmlNode *level_node)
{
	std::unique_ptr<level> rv;

	if (TiXmlElement *element = level_node->ToElement()) {
		std::string foreground, mask;

		for (TiXmlNode *node = element->FirstChild(); node; node = node->NextSibling()) {
			TiXmlElement *e = node->ToElement();
			if (!e)
				continue;

			const char *value = e->Value();

			if (strcmp(value, "foreground") == 0) {
				foreground = e->Attribute("path");
			} else if (strcmp(value, "mask") == 0) {
				mask = e->Attribute("path");
			}
		}

		rv.reset(new level { foreground, mask });
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

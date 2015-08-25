#include <tinyxml.h>

#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/resources.h>
#include <ggl/sprite.h>
#include <ggl/sprite_manager.h>

namespace ggl { namespace res {

const sprite *
sprite_manager::get(const std::string& name) const
{
	auto it = sprite_map_.find(name);

	if (it == sprite_map_.end())
		panic("failed to locate sprite `%s'", name.c_str());

	return it->second.get();
}

void
sprite_manager::load_sprite_sheet(const std::string& path)
{
	auto asset = g_core->get_asset(path);

	std::vector<char> xml(asset->size());
	asset->read(&xml[0], asset->size());

	TiXmlDocument doc;
	doc.Parse(&xml[0]);

	TiXmlElement *root_el = doc.RootElement();

	// texture

	const ggl::texture *tex;

	if (TiXmlElement *texture_el = root_el->FirstChildElement("texture")) {
		tex = res::get_texture(texture_el->Attribute("path"));
	}

	// sprites

	if (TiXmlElement *glyphs_el = root_el->FirstChildElement("sprites")) {
		for (TiXmlNode *node = glyphs_el->FirstChild(); node; node = node->NextSibling()) {
			TiXmlElement *el = node->ToElement();
			if (!el)
				continue;

			// XXX: error checking

			std::string name = el->Attribute("name");

			int x = atoi(el->Attribute("x"));
			int y = atoi(el->Attribute("y"));
			int w = atoi(el->Attribute("w"));
			int h = atoi(el->Attribute("h"));

			auto sp = std::unique_ptr<sprite>(new sprite(tex, x, y, w, h));
			sprite_map_.insert(std::make_pair(name, std::move(sp)));
		}
	}
}

} }

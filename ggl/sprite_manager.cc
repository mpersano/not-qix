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

	auto root_el = doc.RootElement();

	// textures

	std::vector<const ggl::texture *> textures;

	if (auto textures_el  = root_el->FirstChildElement("textures")) {
		for (auto node = textures_el->FirstChild(); node; node = node->NextSibling()) {
			if (auto el = node->ToElement())
				textures.push_back(res::get_texture(el->Attribute("path")));
		}
	}

	// sprites

	if (auto glyphs_el = root_el->FirstChildElement("sprites")) {
		for (auto node = glyphs_el->FirstChild(); node; node = node->NextSibling()) {
			auto el = node->ToElement();
			if (!el)
				continue;

			// TODO: error checking

			std::string name { el->Attribute("name") };

			int tex = atoi(el->Attribute("tex"));

			int u = atoi(el->Attribute("x"));
			int v = atoi(el->Attribute("y"));
			int w = atoi(el->Attribute("w"));
			int h = atoi(el->Attribute("h"));

			auto sp = std::unique_ptr<sprite>(new sprite(textures[tex], u, v, w, h));
			sprite_map_.insert(std::make_pair(name, std::move(sp)));
		}
	}
}

} }

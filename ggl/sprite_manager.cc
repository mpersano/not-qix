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
sprite_manager::load_sprite_sheet(const std::string& name)
{
	auto a = g_core->get_asset(name + ".spr");

	int num_sprites = a->read_uint16();
	(void)a->read_uint8(); // # of sheets

	for (int i = 0; i < num_sprites; i++) {
		std::string sprite_name = a->read_string();

		int sheet_index = a->read_uint8();

		int left = a->read_uint16();
		int top = a->read_uint16();
		int width = a->read_uint16();
		int height = a->read_uint16();

		char texture_path[80];
		sprintf(texture_path, "%s.%03d.png", name.c_str(), sheet_index);

		auto sp = std::unique_ptr<sprite>(new sprite(get_texture(texture_path), left, top, width, height));
		sprite_map_.insert(std::make_pair(sprite_name, std::move(sp)));
	}
}

} }

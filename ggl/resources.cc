#include <string>
#include <memory>
#include <unordered_map>

#include <ggl/panic.h>
#include <ggl/noncopyable.h>
#include <ggl/texture.h>
#include <ggl/font.h>
#include <ggl/sprite_manager.h>
#include <ggl/resources.h>

namespace ggl { namespace res {

namespace {

template <typename ResourceType, typename ImplType>
class resource_manager : private noncopyable
{
public:
	virtual ~resource_manager() = default;

	const ResourceType *get(const std::string& name)
	{
		auto it = resource_map_.find(name);

		if (it == std::end(resource_map_)) {
			it = resource_map_.insert(it, std::make_pair(name, static_cast<ImplType *>(this)->load(name)));
		}

		return it->second.get();
	}

protected:
	std::unordered_map<std::string, std::unique_ptr<ResourceType>> resource_map_;
};

class texture_manager : public resource_manager<texture, texture_manager>
{
public:
	std::unique_ptr<texture> load(const std::string& name)
	{
		return std::unique_ptr<texture>(new texture { image(name) });
	}
} *g_texture_manager;

class font_manager : public resource_manager<font, font_manager>
{
public:
	std::unique_ptr<font> load(const std::string& name)
	{
		return std::unique_ptr<font>(new font { name });
	}
} *g_font_manager;

sprite_manager *g_sprite_manager;

} // (anonymous namespace)

void init()
{
	g_texture_manager = new texture_manager;
	g_font_manager = new font_manager;
	g_sprite_manager = new sprite_manager;
}

const texture *
get_texture(const std::string& name)
{
	return g_texture_manager->get(name);
}

const font *
get_font(const std::string& name)
{
	return g_font_manager->get(name);
}

const sprite *
get_sprite(const std::string& name)
{
	return g_sprite_manager->get(name);
}

void
load_sprite_sheet(const std::string& name)
{
	g_sprite_manager->load_sprite_sheet(name);
}

} }

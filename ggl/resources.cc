#include <string>
#include <memory>
#include <unordered_map>

#include "noncopyable.h"
#include "texture.h"
#include "font.h"
#include "resources.h"

namespace ggl { namespace res {

template <typename ResourceType, typename ImplType>
class resource_manager : private noncopyable
{
public:
	resource_manager() = default;
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
	std::unordered_map<std::string, std::shared_ptr<ResourceType>> resource_map_;
};

class texture_manager : public resource_manager<texture, texture_manager>
{
public:
	std::shared_ptr<texture> load(const std::string& name)
	{
		return std::unique_ptr<texture>(new texture { image(name) });
	}
} *g_texture_manager;

class font_manager : public resource_manager<font, font_manager>
{
public:
	std::shared_ptr<font> load(const std::string& name)
	{
		return std::unique_ptr<font>(new font { name });
	}
} *g_font_manager;

void init()
{
	g_texture_manager = new texture_manager;
	g_font_manager = new font_manager;
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

} }

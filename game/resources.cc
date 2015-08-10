#include <string>
#include <memory>
#include <unordered_map>

#include <ggl/noncopyable.h>
#include <ggl/texture.h>

#include "resources.h"

namespace gp { namespace res {

template <typename ResourceType>
class resource_manager : private ggl::noncopyable
{
public:
	resource_manager() = default;
	virtual ~resource_manager() = default;

	const ResourceType *get(const std::string& name)
	{
		auto it = resource_map_.find(name);

		if (it == std::end(resource_map_)) {
			it = resource_map_.insert(it, std::make_pair(name, load(name)));
		}

		return it->second.get();
	}

protected:
	virtual std::shared_ptr<ResourceType> load(const std::string& name) = 0;

	std::unordered_map<std::string, std::shared_ptr<ResourceType>> resource_map_;
};

class texture_manager : public resource_manager<ggl::texture>
{
public:
	std::shared_ptr<ggl::texture> load(const std::string& name)
	{
		return std::unique_ptr<ggl::texture>(new ggl::texture { ggl::image(name) });
	}
} *g_texture_manager;

void init()
{
	g_texture_manager = new texture_manager;
}

const ggl::texture *
get_texture(const std::string& name)
{
	return g_texture_manager->get(name);
}

} }

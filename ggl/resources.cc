#include <string>
#include <memory>
#include <unordered_map>

#include <ggl/panic.h>
#include <ggl/noncopyable.h>
#include <ggl/texture.h>
#include <ggl/font.h>
#include <ggl/sprite_manager.h>
#include <ggl/program_manager.h>
#include <ggl/action.h>
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

	void load_all();
	void unload_all();
} *g_texture_manager;

class font_manager : public resource_manager<font, font_manager>
{
public:
	std::unique_ptr<font> load(const std::string& name)
	{
		return std::unique_ptr<font>(new font { name });
	}
} *g_font_manager;

class action_manager : public resource_manager<action, action_manager>
{
public:
	std::unique_ptr<action> load(const std::string& name)
	{
		return load_action(name);
	}
} *g_action_manager;

sprite_manager *g_sprite_manager;
program_manager *g_program_manager;

void
texture_manager::load_all()
{
	for (auto& kv : resource_map_)
		kv.second->load();
}

void
texture_manager::unload_all()
{
	for (auto& kv : resource_map_)
		kv.second->unload();
}

} // (anonymous namespace)

void init()
{
	g_texture_manager = new texture_manager;
	g_font_manager = new font_manager;
	g_sprite_manager = new sprite_manager;
	g_program_manager = new program_manager;
	g_action_manager = new action_manager;

	g_program_manager->load_programs("shaders/default.xml");
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

const program *
get_program(const std::string& name)
{
	return g_program_manager->get(name);
}

action_ptr
get_action(const std::string& name)
{
	return g_action_manager->get(name)->clone();
}

void
load_sprite_sheet(const std::string& path)
{
	g_sprite_manager->load_sprite_sheet(path);
}

void
load_programs(const std::string& path)
{
	g_program_manager->load_programs(path);
}

void
unload_gl_resources()
{
	g_texture_manager->unload_all();
	g_program_manager->unload_all();
}

void
load_gl_resources()
{
	g_texture_manager->load_all();
	g_program_manager->load_all();
}

} }

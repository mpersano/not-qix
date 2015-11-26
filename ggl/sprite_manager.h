#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include <ggl/noncopyable.h>

namespace ggl {
class sprite;
};

namespace ggl { namespace res {

class sprite_manager : private noncopyable
{
public:
	const sprite *get(const std::string& name) const;

	void load_sprite_sheet(const std::string& path);

private:
	std::unordered_map<std::string, std::shared_ptr<sprite>> sprite_map_;
};

} }

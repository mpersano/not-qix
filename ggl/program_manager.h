#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include <ggl/noncopyable.h>

namespace ggl {
class program;
};

namespace ggl { namespace res {

class program_manager : private noncopyable
{
public:
	const program *get(const std::string& name) const;

	void load_programs(const std::string& path);

	void unload_all();
	void load_all();

private:
	std::unordered_map<std::string, std::shared_ptr<program>> program_map_;
};

} }

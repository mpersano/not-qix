#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include "texture.h"

namespace ggl {

class texture_manager
{
	using dict_type = std::unordered_map<std::string, std::unique_ptr<texture>>;
	using dict_value_type = dict_type::value_type;

public:
	static texture_manager& get_instance();

	const texture *get(const char *source);

	void load_all();

private:
	texture_manager();

	dict_type texture_dict_;
};

}

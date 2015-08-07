#include <cstdio>
#include <algorithm>

#include "image.h"
#include "texture_manager.h"

namespace ggl {

texture_manager::texture_manager()
{ }

texture_manager &
texture_manager::get_instance()
{
	static texture_manager tm;
	return tm;
}

const texture *
texture_manager::get(const char *source)
{
	auto it = texture_dict_.find(source);

	if (it == texture_dict_.end()) {
		it = texture_dict_.insert(it, dict_value_type(source, std::unique_ptr<texture>(new texture { image(source) })));
	}

	return it->second.get();
}

}

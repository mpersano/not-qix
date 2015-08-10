#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ggl {

enum class pixel_type { GRAY, GRAY_ALPHA, RGB, RGB_ALPHA };

unsigned
get_pixel_size(pixel_type type);

struct image
{
	image(const std::string& path);
	image(unsigned width, unsigned height, pixel_type type);

	unsigned row_stride() const
	{ return width*pixel_size(); }

	unsigned pixel_size() const
	{ return get_pixel_size(type); }

	unsigned width;
	unsigned height;
	pixel_type type;
	std::vector<uint8_t> data;
};

} // ggl

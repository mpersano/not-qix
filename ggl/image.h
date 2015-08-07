#pragma once

#include <cstdint>
#include <vector>

namespace ggl {

struct image
{
	enum class pixel_type { GRAY, GRAY_ALPHA, RGB, RGB_ALPHA };

	image(const char *path);
	image(unsigned width, unsigned height, pixel_type type);

	size_t get_pixel_size() const;
	size_t get_row_stride() const;

	unsigned width;
	unsigned height;
	pixel_type type;
	std::vector<uint8_t> data;
};

} // ggl

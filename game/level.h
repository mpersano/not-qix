#pragma once

#include <vector>

namespace ggl {
class texture;
}

static const int CELL_SIZE = 16;

class level
{
public:
	level(const char *background, const char *mask);

	const ggl::texture *background_texture;
	const ggl::texture *mask_texture;
	int grid_rows, grid_cols;
	std::vector<int> silhouette;
	int silhouette_pixels;
};

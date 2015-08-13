#pragma once

#include <vector>
#include <memory>

namespace ggl {
class texture;
}

static const int CELL_SIZE = 16;

class level
{
public:
	level(const std::string& background, const std::string& mask);

	const ggl::texture *background_texture;
	const ggl::texture *mask_texture;
	int grid_rows, grid_cols;
	std::vector<int> silhouette;
	int silhouette_pixels;
};

extern std::vector<std::unique_ptr<level>> g_levels;

void
init_levels();

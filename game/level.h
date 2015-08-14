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
	level(const std::string& fg_path, const std::string& bg_path, const std::string& mask_path);

	const ggl::texture *fg_texture;
	const ggl::texture *bg_texture;

	int grid_rows, grid_cols;
	std::vector<int> silhouette;
	unsigned silhouette_pixels;
};

extern std::vector<std::unique_ptr<level>> g_levels;

void
init_levels();

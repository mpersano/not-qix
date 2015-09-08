#pragma once

#include <string>
#include <vector>
#include <memory>

#include <wchar.h>

namespace ggl {
class texture;
}

static const int CELL_SIZE = 8;

class level
{
public:
	level(const std::string& fg_path, const std::string& bg_path, const std::string& mask_path, const std::string& portrait_path);

	std::basic_string<wchar_t> name;
	const ggl::texture *fg_texture;
	const ggl::texture *bg_texture;
	const ggl::texture *portrait_texture;

	int grid_rows, grid_cols;
	std::vector<int> silhouette;
	unsigned silhouette_pixels;
};

extern std::vector<std::unique_ptr<level>> g_levels;

void
init_levels();

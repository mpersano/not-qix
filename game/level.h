#pragma once

#include <ggl/texture.h>

#include <memory>

#include "common.h"

class level
{
public:
	level(const char *background, const char *mask);

	std::unique_ptr<ggl::texture> background_texture;
	std::unique_ptr<ggl::texture> mask_texture;
	int silhouette[GRID_ROWS*GRID_COLS];
	int silhouette_pixels;
};

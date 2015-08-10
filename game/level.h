#pragma once

#include <ggl/texture.h>

#include "common.h"

class level
{
public:
	level(const char *background, const char *mask);

	const ggl::texture *background_texture;
	const ggl::texture *mask_texture;
	int silhouette[GRID_ROWS*GRID_COLS];
	int silhouette_pixels;
};

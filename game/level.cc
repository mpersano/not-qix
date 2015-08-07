#include <cassert>
#include <algorithm>

#include "level.h"

level::level(const char *background, const char *mask)
{
	ggl::image background_pm(background);
	ggl::image mask_pm(mask);

	assert(background_pm.width == mask_pm.width);
	assert(background_pm.height == mask_pm.height);
	assert(mask_pm.height%GRID_ROWS == 0);
	assert(mask_pm.width%GRID_COLS == 0);

	// textures

	background_texture.reset(new ggl::texture { background_pm });
	mask_texture.reset(new ggl::texture { mask_pm });

	// silhouette

	const size_t row_stride = mask_pm.get_row_stride();
	const size_t pixel_size = mask_pm.get_pixel_size();
	const uint8_t *mask_pixels = &mask_pm.data[0];

	int block_height = mask_pm.height/GRID_ROWS;
	int block_width = mask_pm.width/GRID_COLS;

	for (int r = 0; r < GRID_ROWS; r++) {
		for (int c = 0; c < GRID_COLS; c++) {
			int s = 0;

			auto *p = &mask_pixels[r*block_height*row_stride + c*block_width*pixel_size];

			for (int i = 0; i < block_height; i++) {
				for (int j = 0; j < block_width; j++) {
					s += *p != 0;
					p += pixel_size;
				}

				p += row_stride - block_width*pixel_size;
			}

			silhouette[(GRID_ROWS - 1 - r)*GRID_COLS + c] = s; // flipped...
		}
	}

	silhouette_pixels = std::accumulate(std::begin(silhouette), std::end(silhouette), 0);
}

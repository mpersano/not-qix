#include <cassert>
#include <algorithm>

#include <ggl/texture.h>
#include <ggl/resources.h>

#include "level.h"

level::level(const char *background, const char *mask)
: background_texture { ggl::res::get_texture(background) }
, mask_texture { ggl::res::get_texture(mask) }
{
	assert(background_texture->orig_width == mask_texture->orig_width);
	assert(background_texture->orig_height == mask_texture->orig_height);
	assert(mask_texture->orig_width%CELL_SIZE == 0);
	assert(mask_texture->orig_height%CELL_SIZE == 0);

	const unsigned row_stride = mask_texture->row_stride();
	const unsigned pixel_size = mask_texture->pixel_size();
	const uint8_t *mask_pixels = &mask_texture->data[0];

	grid_rows = mask_texture->orig_height/CELL_SIZE;
	grid_cols = mask_texture->orig_width/CELL_SIZE;

	silhouette.resize(grid_rows*grid_cols);

	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_cols; c++) {
			int s = 0;

			auto *p = &mask_pixels[r*CELL_SIZE*row_stride + c*CELL_SIZE*pixel_size];

			for (int i = 0; i < CELL_SIZE; i++) {
				for (int j = 0; j < CELL_SIZE; j++) {
					s += *p != 0;
					p += pixel_size;
				}

				p += row_stride - CELL_SIZE*pixel_size;
			}

			silhouette[r*grid_cols + c] = s;
		}
	}

	silhouette_pixels = std::accumulate(std::begin(silhouette), std::end(silhouette), 0);
}

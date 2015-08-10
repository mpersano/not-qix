#include <cassert>
#include <algorithm>

#include "resources.h"
#include "level.h"

level::level(const char *background, const char *mask)
: background_texture { gp::res::get_texture(background) }
, mask_texture { gp::res::get_texture(mask) }
{
	assert(background_texture->orig_width == mask_texture->orig_width);
	assert(background_texture->orig_height == mask_texture->orig_height);
	assert(mask_texture->orig_height%GRID_ROWS == 0);
	assert(mask_texture->orig_height%GRID_COLS == 0);

	const unsigned row_stride = mask_texture->row_stride();
	const unsigned pixel_size = mask_texture->pixel_size();
	const uint8_t *mask_pixels = &mask_texture->data[0];

	unsigned block_height = mask_texture->orig_height/GRID_ROWS;
	unsigned block_width = mask_texture->orig_width/GRID_COLS;

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

			silhouette[r*GRID_COLS + c] = s;
		}
	}

	silhouette_pixels = std::accumulate(std::begin(silhouette), std::end(silhouette), 0);
}

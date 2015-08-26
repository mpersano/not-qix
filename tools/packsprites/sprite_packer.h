#pragma once

#include <vector>

#include "pixmap.h"

class sprite_base;

void pack_sprites(std::vector<sprite_base *>& sprites,
			const std::string& sheet_name,
			int sheet_width, int sheet_height,
			int border,
			pixmap::type color_type,
			const std::string& texture_path_base);

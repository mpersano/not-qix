#pragma once

#include <vector>

#include "pixmap.h"

class sprite_base;
class node;

class sprite_packer
{
public:
	sprite_packer();

	void pack(std::vector<sprite_base *>& sprites, const char *sheet_name, pixmap::type color_type);

	void set_border(size_t border);
	void set_sheet_size(size_t width, size_t height);

protected:
	void write_sprite_sheet(pixmap& pm, const node *root);
	void write_sprite_sheet(const char *name, pixmap::type color_type, const node *root);

	size_t sheet_width_, sheet_height_;
	size_t border_;
};

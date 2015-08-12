#pragma once

#include <vector>

#include "pixmap.h"

class sprite_base;

class sprite_packer
{
public:
	static sprite_packer *make(bool multi);

	virtual void pack(std::vector<sprite_base *>& sprites, const char *sheet_name, pixmap::type color_type) = 0;

	void set_border(size_t border);
	void set_sheet_size(size_t width, size_t height);

protected:
	sprite_packer();
	virtual ~sprite_packer();

	size_t sheet_width_, sheet_height_;
	size_t border_;
};

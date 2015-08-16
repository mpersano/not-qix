#pragma once

#include <string>

#include "sprite_base.h"

struct sprite : sprite_base
{
	sprite(const char *name, pixmap *pm);

	void serialize(file_writer& fw) const;

	std::string name_;
};

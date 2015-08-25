#pragma once

#include <string>

#include "sprite_base.h"

struct sprite : sprite_base
{
	sprite(const char *name, pixmap *pm);

	void serialize(FILE *out, const rect& rc, int border) const override;

	std::string name_;
};

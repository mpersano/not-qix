#pragma once

#include <string>

#include "sprite_base.h"

struct sprite : sprite_base
{
	sprite(const char *name, pixmap *pm);

	void serialize(TiXmlElement *el) const override;

	std::string name_;
};

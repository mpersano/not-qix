#pragma once

#include <vector>

#include "file_writer.h"
#include "pixmap.h"

struct sprite_base
{
	sprite_base(pixmap *pm);
	virtual ~sprite_base();

	size_t width() const
	{ return pm_->get_width(); }

	size_t height() const
	{ return pm_->get_height(); }

	virtual void serialize(file_writer& fw) const = 0;

	pixmap *pm_;
};

#include <cassert>

#include "rect.h"

std::pair<rect, rect>
rect::split_vert(int c) const
{
	assert(c < width_);
	return std::pair<rect, rect>(rect(left_, top_, c, height_), rect(left_ + c, top_, width_ - c, height_));
}

std::pair<rect, rect>
rect::split_horiz(int r)
{
	assert(r < height_);
	return std::pair<rect, rect>(rect(left_, top_, width_, r), rect(left_, top_ + r, width_, height_ - r));
}

#pragma once

#include <utility>

struct rect
{
	rect(int left, int top, int width, int height)
	: left_(left), top_(top), width_(width), height_(height)
	{ }

	int left_, top_, width_, height_;

	std::pair<rect, rect> split_vert(int c) const;
	std::pair<rect, rect> split_horiz(int r);
};

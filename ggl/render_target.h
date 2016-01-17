#pragma once

namespace ggl {

class render_target
{
public:
	render_target(int width, int height)
	: width_ { width }
	, height_ { height }
	{ }

	virtual ~render_target()
	{ }

	int get_width() const
	{ return width_; }

	int get_height() const
	{ return height_; }

	virtual void bind() const = 0;

protected:
	int width_, height_;
};

}

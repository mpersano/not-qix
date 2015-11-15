#pragma once

class app_state
{
public:
	app_state(int width, int height)
	: width_(width), height_(height)
	{ }

	virtual ~app_state() = default;

	virtual void draw() = 0;
	virtual void update() = 0;

protected:
	int width_, height_;
};

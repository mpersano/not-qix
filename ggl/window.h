#pragma once

#include <vector>

namespace ggl {

enum {
	DPAD_UP = 1,
	DPAD_DOWN = 2,
	DPAD_LEFT = 4,
	DPAD_RIGHT = 8,
	DPAD_BUTTON1 = 16,
	DPAD_BUTTON2 = 32,
};

class window
{
public:
	window(int width, int height);
	virtual ~window();

	virtual void run() = 0;
	virtual void update_and_render(float dt) = 0;

protected:
	int width_, height_;
	unsigned dpad_state_;
};

}

#pragma once

namespace ggl {

class app
{
public:
	virtual ~app() = default;

	virtual void init(int width, int height) = 0;
	virtual void update_and_render(float dt) = 0;

	virtual void on_pointer_down(int x, int y) = 0;
	virtual void on_pointer_up(int x, int y) = 0;
	virtual void on_pointer_move(int x, int y) = 0;
};

}

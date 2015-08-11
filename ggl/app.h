#pragma once

namespace ggl {

class app
{
public:
	app() = default;
	virtual ~app() = default;

	virtual void init(int width, int height) = 0;
	virtual void update_and_render(float dt) = 0;
};

}

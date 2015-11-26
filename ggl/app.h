#pragma once

namespace ggl {

class app
{
public:
	virtual ~app() = default;

	virtual void init(unsigned width, unsigned height) = 0;
	virtual void update_and_render(float dt) = 0;
};

}

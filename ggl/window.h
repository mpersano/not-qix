#pragma once

#include <ggl/render_target.h>

namespace ggl {

class window : public render_target
{
public:
	window();

	void bind() const override;
};

}

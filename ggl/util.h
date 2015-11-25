#pragma once

#include <ggl/gl.h>

namespace ggl {

template <GLenum SFactor, GLenum DFactor>
struct enable_blend
{
	enable_blend()
	{
		glEnable(GL_BLEND);
		glBlendFunc(SFactor, DFactor);
	}

	~enable_blend()
	{
		glDisable(GL_BLEND);
	}
};

using enable_alpha_blend = enable_blend<GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA>;
using enable_additive_blend = enable_blend<GL_ONE, GL_ONE>;

}

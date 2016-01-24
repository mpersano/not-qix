#include <ggl/rgba.h>
#include <ggl/render.h>

#include "debuggfx.h"

void
draw_circle(const vec2f& center, float radius, float width)
{
	static const int NUM_SEGMENTS = 15;

	float a = 0;
	float da = 2.*M_PI/NUM_SEGMENTS;

	for (int i = 0; i < NUM_SEGMENTS; i++) {
		const float s0 = sinf(a);
		const float c0 = cosf(a);

		const vec2f p0 = center + (radius - .5f*width)*vec2f { s0, c0 };
		const vec2f p1 = center + (radius + .5f*width)*vec2f { s0, c0 };

		const float s1 = sinf(a + da);
		const float c1 = cosf(a + da);

		const vec2f p2 = center + (radius + .5f*width)*vec2f { s1, c1 };
		const vec2f p3 = center + (radius - .5f*width)*vec2f { s1, c1 };

		ggl::render::draw(ggl::quad { p0, p1, p2, p3 }, 30.f);

		a += da;
	}
}

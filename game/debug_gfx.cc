#include <ggl/vertex_array.h>

#include "debug_gfx.h"

void
draw_circle(const vec2f& center, float radius)
{
	static const int NUM_SEGS = 13;

	float a = 0;
	const float da = 2.f*M_PI/NUM_SEGS;

	ggl::vertex_array_flat<GLfloat, 2> va;

	for (int i = 0; i < NUM_SEGS; i++) {
		vec2f p = center + vec2f { cosf(a), sinf(a) }*radius;
		va.push_back({ p.x, p.y });
		a += da;
	}

	va.draw(GL_LINE_LOOP);
}

void
draw_line(const vec2f& p0, const vec2f& p1)
{
	(ggl::vertex_array_flat<GLfloat, 2>{ { p0.x, p0.y }, { p1.x, p1.y } }).draw(GL_LINES);
}

void
draw_box(const vec2f& p0, const vec2f& p1)
{
	(ggl::vertex_array_flat<GLfloat, 2>
		{ { p0.x, p0.y },
		  { p0.x, p1.y },
		  { p1.x, p1.y },
		  { p1.x, p0.y } }).draw(GL_LINE_LOOP);
}

#include <ggl/gl.h>

#include "explosion.h"

namespace {

const int TTL = 90;

} // (anonymous namespace)

explosion::explosion(const vec2f& pos)
: pos_ { pos }
, radius_ { 5 }
, tics_ { 0 }
{ }

bool
explosion::update()
{
	radius_ *= 1.05f;

	return ++tics_ < TTL;
}

void
explosion::draw() const
{
	static const int NUM_SEGS = 13;

	float a = 0;
	const float da = 2.f*M_PI/NUM_SEGS;

	glColor4f(1, 1, 0, 1);

	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < NUM_SEGS; i++) {
		vec2f p = pos_ + vec2f { cosf(a), sinf(a) }*radius_;
		glVertex2f(p.x, p.y);
		a += da;
	}

	glEnd();
}

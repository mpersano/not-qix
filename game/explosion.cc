#include <ggl/gl.h>

#include "debug_gfx.h"
#include "explosion.h"

namespace {

const int TTL = 30;

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
	glColor4f(1, 1, 0, 1);
	draw_circle(pos_, radius_);
}

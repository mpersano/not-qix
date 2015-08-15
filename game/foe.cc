#include <cmath>

#include "game.h"
#include "foe.h"

namespace {

vec2f
seg_closest_point(const vec2f& v0, const vec2f& v1, const vec2f& p)
{
	vec2f d = v1 - v0;
	vec2f v = normalized(v1 - v0);

	float t = dot(p - v0, v);

	if (t < 0)
		return v0;
	else if (t > length(d))
		return v1;
	else
		return v0 + t*v;
}

}

foe::foe(game& g)
: game_(g)
, position_ { 100, 100 }
, radius_ { 30 }
, dir_ { normalized(vec2f { 1.f, .5f }) }
, speed_ { 3 }
{ }

void
foe::draw() const
{
	static const int NUM_SEGS = 13;

	glDisable(GL_TEXTURE_2D);
	glColor4f(1, 1, 0, 1);

	float a = 0;
	const float da = 2.f*M_PI/NUM_SEGS;

	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < NUM_SEGS; i++) {
		vec2f p = position_ + vec2f { cosf(a), sinf(a) }*radius_;
		glVertex2f(p.x, p.y);
		a += da;
	}

	glEnd();
}

bool
foe::update(float dt)
{
	// update position

	position_ += speed_*dir_;

	// collide against edge

	const auto& border = game_.border;

	static const float FUDGE = 2.;

	bool collided;

	do {
		collided = false;

		for (size_t i = 0; i < border.size(); i++) {
			const vec2f v0 = border[i]*CELL_SIZE;
			const vec2f v1 = border[(i + 1)%border.size()]*CELL_SIZE;

			vec2f c = seg_closest_point(v0, v1, position_);
			vec2f d = position_ - c;

			float dist = length(d);

			if (dist < radius_) {
				position_ += ((radius_ - dist) + FUDGE)*normalized(d);
				vec2f n = normalized(position_ - c);
				dir_ -= 2.f*dot(dir_, n)*n;

				collided = true;
			}
		}
	} while (collided);

	return true;
}

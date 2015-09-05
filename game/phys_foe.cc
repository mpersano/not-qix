#include <cmath>

#include "game.h"
#include "phys_foe.h"

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

phys_foe::phys_foe(game& g, const vec2f& pos, const vec2f& dir, float speed, float radius)
: foe { g }
, pos_ { pos }
, dir_ { dir }
, speed_ { speed }
, radius_ { radius }
{ }

void
phys_foe::update_position()
{
	// update position

	pos_ += speed_*dir_;

	// collide against border

	const auto& border = game_.border;

	const int height = game_.grid_rows*CELL_SIZE;
	const int width = game_.grid_cols*CELL_SIZE;

	bool collided;

	do {
		collided = false;

		for (size_t i = 0; i < border.size(); i++) {
			const vec2f v0 = border[i]*CELL_SIZE;
			const vec2f v1 = border[(i + 1)%border.size()]*CELL_SIZE;

			if (collide_against_edge(v0, v1))
				collided = true;
		}

		if (collide_against_edge({ 0, 0 }, { 0, height }))
			collided = true;

		if (collide_against_edge({ 0, height }, { width, height }))
			collided = true;

		if (collide_against_edge({ width, height }, { width, 0 }))
			collided = true;

		if (collide_against_edge({ width, 0 }, { 0, 0 }))
			collided = true;
	} while (collided);
}

bool
phys_foe::collide_against_edge(const vec2f& v0, const vec2f& v1)
{
	vec2f c = seg_closest_point(v0, v1, pos_);
	vec2f d = pos_ - c;

	float dist = length(d);

	if (dist < radius_) {
		static const float FUDGE = 2.;
		pos_ += ((radius_ - dist) + FUDGE)*normalized(d);
		vec2f n = normalized(pos_ - c);
		dir_ -= 2.f*dot(dir_, n)*n;

		return true;
	}

	return false;
}

void
phys_foe::rotate_to_player()
{
	vec2f n { -dir_.y, dir_.x };

	float da = .025f*dot(n, normalized(vec2f(game_.get_player_world_position()) - pos_));

	const float c = cosf(da);
	const float s = sinf(da);

	vec2f next_dir { dot(dir_, vec2f { c, -s }), dot(dir_, vec2f { s, c }) };
	dir_ = next_dir;
}

void
phys_foe::set_speed(float speed)
{
	speed_ = speed;
}

bool
phys_foe::intersects(const vec2i& from, const vec2i& to) const
{
	return length(pos_ - seg_closest_point(vec2f(from), vec2f(to), pos_)) < radius_;
}

vec2f
phys_foe::get_position() const
{
	return pos_;
}

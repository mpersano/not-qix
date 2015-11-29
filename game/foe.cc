#include <cmath>

#include <ggl/vec2_util.h>

#include "game.h"
#include "foe.h"

foe::foe(game& g, const vec2f& pos, float radius)
: entity { g }
, pos_ { pos }
, dir_ { 1, 0 }
, speed_ { 0 }
, radius_ { radius }
{ }

void
foe::update_position()
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
foe::collide_against_edge(const vec2f& v0, const vec2f& v1)
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
foe::rotate_to_player()
{
	vec2f n { -dir_.y, dir_.x };

	float da = .025f*dot(n, normalized(vec2f(game_.get_player_world_position()) - pos_));

	const float c = cosf(da);
	const float s = sinf(da);

	vec2f next_dir { dot(dir_, vec2f { c, -s }), dot(dir_, vec2f { s, c }) };
	dir_ = next_dir;
}

void
foe::rotate(float a)
{
	dir_ = dir_.rotate(a);
}

void
foe::set_direction(const vec2f& dir)
{
	dir_ = normalized(dir);
}

void
foe::set_speed(float speed)
{
	speed_ = speed;
}

bool
foe::intersects(const vec2i& from, const vec2i& to) const
{
	return length(pos_ - seg_closest_point(vec2f(from), vec2f(to), pos_)) < radius_ || intersects_children(from, to);
}

bool
foe::intersects(const vec2i& center, float radius) const
{
	return length(pos_ - vec2f(center)) < radius_ + radius || intersects_children(center, radius);
}

vec2f
foe::get_position() const
{
	return pos_;
}

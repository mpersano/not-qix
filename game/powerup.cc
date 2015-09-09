#include <cmath>
#include <cassert>
#include <algorithm>

#include <ggl/gl.h>

#include "game.h"
#include "powerup.h"

namespace {

const int RADIUS = 12;
const float SPEED = 1.5;

};

powerup::powerup(game& g, const vec2f& pos, const vec2f& dir)
: foe { g }
, pos_ { pos }
, dir_ { dir }
, state_ { state::MOVING }
{ }

void
powerup::draw() const
{
	glColor4f(1, 0, 0, 1);

	static const int NUM_SEGS = 13;

	float a = 0;
	const float da = 2.f*M_PI/NUM_SEGS;

	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < NUM_SEGS; i++) {
		vec2f p = pos_ + vec2f { cosf(a), sinf(a) }*RADIUS;
		glVertex2f(p.x, p.y);
		a += da;
	}

	glEnd();

	glColor4f(1, 1, 1, 1);

	glBegin(GL_LINES);

	glVertex2f(pos_.x - 5, pos_.y - 5);
	glVertex2f(pos_.x + 5, pos_.y + 5);

	glVertex2f(pos_.x + 5, pos_.y - 5);
	glVertex2f(pos_.x - 5, pos_.y + 5);
	glEnd();
}

bool
powerup::update()
{
	switch (state_) {
		case state::MOVING:
			update_moving();
			break;

		case state::SLIDING:
			update_sliding();
			break;
	}

	return true;
}

void
powerup::update_moving()
{
	auto next_pos = pos_ + SPEED*dir_;

	// collision with filled area

	const int c0 = static_cast<int>(pos_.x/CELL_SIZE);
	const int c1 = static_cast<int>(next_pos.x/CELL_SIZE);

	if (c0 != c1) {
		int cm = c0 < c1 ? c1 : c0;
		float xm = cm*CELL_SIZE;
		assert((xm >= pos_.x && xm <= next_pos.x) || (xm >= next_pos.x && xm <= pos_.x));
		float ym = (next_pos.y - pos_.y)*(xm - pos_.x)/(next_pos.x - pos_.x) + pos_.y;
		const int r = static_cast<int>(ym/CELL_SIZE);

		if (game_.grid[r*game_.grid_cols + c0] != game_.grid[r*game_.grid_cols + c1]) {
			pos_ = vec2f { xm, ym };
			state_ = state::SLIDING;
			dir_ = vec2f { 0, 1 };
			return;
		}
	}

	const int r0 = static_cast<int>(pos_.y/CELL_SIZE);
	const int r1 = static_cast<int>(next_pos.y/CELL_SIZE);

	if (r0 != r1) {
		int rm = r0 < r1 ? r1 : r0;
		float ym = rm*CELL_SIZE;
		assert((ym >= pos_.y && ym <= next_pos.y) || (ym >= next_pos.y && ym <= pos_.y));
		float xm = (next_pos.x - pos_.x)*(ym - pos_.y)/(next_pos.y - pos_.y) + pos_.x;
		const int c = static_cast<int>(xm/CELL_SIZE);

		if (game_.grid[r0*game_.grid_cols + c] != game_.grid[r1*game_.grid_cols + c]) {
			pos_ = vec2f { xm, ym };
			state_ = state::SLIDING;
			dir_ = vec2f { 1, 0 };
			return;
		}
	}

	pos_ = next_pos;

	// collide with viewport limits

	const vec2i v0 = -game_.offset;
	const vec2i v1 {
		std::min(game_.grid_cols*CELL_SIZE, v0.x + game_.viewport_width),
			std::min(game_.grid_rows*CELL_SIZE, v0.y + game_.viewport_height) };

	if (dir_.x < 0 && pos_.x < v0.x + RADIUS)
		dir_.x = -dir_.x;

	if (dir_.x > 0 && pos_.x > v1.x - RADIUS)
		dir_.x = -dir_.x;

	if (dir_.y < 0 && pos_.y < v0.y + RADIUS)
		dir_.y = -dir_.y;

	if (dir_.y > 0 && pos_.y > v1.y - RADIUS)
		dir_.y = -dir_.y;
}

void
powerup::update_sliding()
{
	auto next_pos = pos_ + SPEED*dir_;

	// collision with filled area

	const auto& grid = game_.grid;
	const auto grid_rows = game_.grid_rows;
	const auto grid_cols = game_.grid_cols;

	const int c0 = static_cast<int>(pos_.x/CELL_SIZE);
	const int c1 = static_cast<int>(next_pos.x/CELL_SIZE);

	const int r0 = static_cast<int>(pos_.y/CELL_SIZE);
	const int r1 = static_cast<int>(next_pos.y/CELL_SIZE);

	if (c0 != c1) {
		int c = c0 < c1 ? c1 : c0;
		int r = r0;

		if (grid[r*grid_cols + c1] == grid[(r - 1)*grid_cols + c1]) {
			// change direction

			next_pos = vec2f { c, r }*CELL_SIZE;

			if (grid[r*grid_cols + c - 1] != grid[r*grid_cols + c]) {
				dir_ = vec2f { 0, 1 };
			} else if (grid[(r - 1)*grid_cols + c - 1] != grid[(r - 1)*grid_cols + c]) {
				dir_ = vec2f { 0, -1 };
			} else {
				assert(0);
			}
		}
	} else if (r0 != r1) {
		int r = r0 < r1 ? r1 : r0;
		int c = c0;

		if (grid[r1*grid_cols + c - 1] == grid[r1*grid_cols + c]) {
			// change direction

			next_pos = vec2f { c, r }*CELL_SIZE;

			if (grid[r*grid_cols + c - 1] != grid[(r - 1)*grid_cols + c - 1]) {
				dir_ = vec2f { -1, 0 };
			} else if (grid[r*grid_cols + c] != grid[(r - 2)*grid_cols + c]) {
				dir_ = vec2f { 1, 0 };
			} else {
				assert(0);
			}
		}
	}

	pos_ = next_pos;
}

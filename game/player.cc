#include <algorithm>
#include <cassert>

#include <ggl/sprite.h>
#include <ggl/resources.h>
#include <ggl/dpad_button.h>

#include "game.h"
#include "powerup.h"
#include "player.h"

namespace {
const float RADIUS = 2;
};

player::player(game& g)
: game_ { g }
{
	for (int i = 0; i < NUM_FRAMES; i++) {
		char name[80];

		sprintf(name, "player-core-%02d.png", i);
		sprites_core_[i] = ggl::res::get_sprite(name);

		sprintf(name, "player-shield-%02d.png", i);
		sprites_shield_[i] = ggl::res::get_sprite(name);
	}
}

void
player::reset(const vec2i& pos)
{
	pos_ = pos;
	set_state(state::IDLE);
	tic_ = 0;
}

void
player::move(direction dir, bool button)
{
	switch (state_) {
		case state::IDLE:
			move_slide(dir);

			if (state_ != state::SLIDING && button)
				move_extend(dir);
			break;

		case state::EXTENDING_IDLE:
			if (button)
				move_extend(dir);
			break;
	}
}

void
player::move_extend(direction dir)
{
	const int grid_cols = game_.grid_cols;
	const int grid_rows = game_.grid_rows;

	auto *p = &game_.grid[pos_.y*grid_cols + pos_.x];

	auto extend_to = [&](const vec2i& where)
		{
			auto it = std::find(std::begin(extend_trail_), std::end(extend_trail_), where);

			if (it == extend_trail_.end() || it + 1 == extend_trail_.end()) {
				next_pos_ = where;

				if (extend_trail_.empty() || extend_trail_.back() != where)
					extend_trail_.push_back(pos_);

				set_state(state::EXTENDING);
			}
		};

	switch (dir) {
		case direction::UP:
			if (pos_.y < grid_rows - 1) {
				if (!p[0] && !p[-1])
					extend_to(pos_ + vec2i { 0, 1 });
			}
			break;

		case direction::DOWN:
			if (pos_.y > 1) {
				if (!p[-grid_cols] && !p[-grid_cols - 1])
					extend_to(pos_ + vec2i { 0, -1 });
			}
			break;

		case direction::LEFT:
			if (pos_.x > 1) {
				if (!p[-grid_cols - 1] && !p[-1])
					extend_to(pos_ + vec2i { -1, 0 });
			}
			break;

		case direction::RIGHT:
			if (pos_.x < grid_cols - 1) {
				if (!p[-grid_cols] && !p[0])
					extend_to(pos_ + vec2i { 1, 0 });
			}
			break;
	}
}

void
player::move_slide(direction dir)
{
	const int grid_cols = game_.grid_cols;
	const int grid_rows = game_.grid_rows;

	auto *p = &game_.grid[pos_.y*grid_cols + pos_.x];

	switch (dir) {
		case direction::UP:
			if (p[-1] != p[0]) {
				next_pos_ = pos_ + vec2i { 0, 1 };
				set_state(state::SLIDING);
			}
			break;

		case direction::DOWN:
			if (p[-grid_cols - 1] != p[-grid_cols]) {
				next_pos_ = pos_ + vec2i { 0, -1 };
				set_state(state::SLIDING);
			}
			break;

		case direction::LEFT:
			if (p[-grid_cols - 1] != p[-1]) {
				next_pos_ = pos_ + vec2i { -1, 0 };
				set_state(state::SLIDING);
			}
			break;

		case direction::RIGHT:
			if (p[-grid_cols] != p[0]) {
				next_pos_ = pos_ + vec2i { 1, 0 };
				set_state(state::SLIDING);
			}
			break;
	}
}

void
player::update(unsigned dpad_state)
{
	++tic_;

	auto dpad_button_pressed = [=](ggl::dpad_button button)
		{
			return dpad_state & (1u << button);
		};

	bool button = dpad_button_pressed(ggl::BUTTON1);

	if (dpad_button_pressed(ggl::UP))
		move(direction::UP, button);

	if (dpad_button_pressed(ggl::DOWN))
		move(direction::DOWN, button);

	if (dpad_button_pressed(ggl::LEFT))
		move(direction::LEFT, button);

	if (dpad_button_pressed(ggl::RIGHT))
		move(direction::RIGHT, button);

	const int grid_cols = game_.grid_cols;
	const int grid_rows = game_.grid_rows;

	switch (state_) {
		case state::IDLE:
			break;

		case state::EXTENDING_IDLE:
			if (!button) {
				assert(!extend_trail_.empty());
				next_pos_ = extend_trail_.back();
				set_state(state::EXTENDING);
			}

			check_foe_collisions();
			break;

		case state::SLIDING:
			if (++state_tics_ >= SLIDE_TICS) {
				pos_ = next_pos_;
				set_state(state::IDLE);
			}
			break;

		case state::EXTENDING:
			if (++state_tics_ >= SLIDE_TICS) {
				pos_ = next_pos_;

				if (!extend_trail_.empty() && extend_trail_.back() == pos_)
					extend_trail_.pop_back();

				if (extend_trail_.empty()) {
					set_state(state::IDLE);
				} else {
					assert(pos_.x != 0 && pos_.x != grid_cols);
					assert(pos_.y != 0 && pos_.y != grid_rows);

					auto *p = &game_.grid[pos_.y*grid_cols + pos_.x];

					if (p[0] || p[-1] || p[-grid_cols] || p[-grid_cols - 1]) {
						extend_trail_.push_back(pos_);
						game_.fill_grid(extend_trail_);

						extend_trail_.clear();
						set_state(state::IDLE);
					} else {
						if (button) {
							state_ = state::EXTENDING_IDLE;
						} else {
							// move back if button not pressed
							next_pos_ = extend_trail_.back();
							set_state(state::EXTENDING);
						}
					}
				}
			}

			check_foe_collisions();
			break;
	}
}

void
player::check_foe_collisions()
{
	if (extend_trail_.size() > 1) {
		auto& entities = game_.entities;

		auto it = std::find_if(
				std::begin(entities),
				std::end(entities),
				[this](std::unique_ptr<entity>& f)
					{
						for (size_t i = 0; i < extend_trail_.size() - 1; i++) {
							const vec2i v0 = extend_trail_[i]*CELL_SIZE;
							const vec2i v1 = extend_trail_[i + 1]*CELL_SIZE;

							if (f->intersects(v0, v1))
								return true;
						}

						if (f->intersects(extend_trail_.back()*CELL_SIZE, get_position()))
							return true;

						if (f->intersects(get_position(), RADIUS))
							return true;

						return false;
					});

		if (it != std::end(entities))
			die();
	}
}

void
player::die()
{
	printf("death!\n");

	int num_powerups = 5;

	float da = 2.f*M_PI/num_powerups;
	float a = .5f*da;

	for (int i = 0; i < num_powerups; i++) {
		const float c = cosf(a);
		const float s = sinf(a);

		game_.add_entity(std::unique_ptr<entity>(new powerup(game_, get_position(), vec2f { c, s })));

		a += da;
	}

	pos_ = extend_trail_.front();
	extend_trail_.clear();

	state_ = state::IDLE;
}

void
player::set_state(state next_state)
{
	state_ = next_state;
	state_tics_ = 0;
}

void
player::draw() const
{
	glDisable(GL_TEXTURE_2D);

	// trail

	if (state_ == state::EXTENDING || state_ == state::EXTENDING_IDLE) {
		static const int TRAIL_RADIUS = 1;

		glColor4f(1, 1, 0, 1);

		if (extend_trail_.size() > 1) {
			ggl::vertex_array_flat<GLshort, 2> va;

			// first
			{
				auto& v0 = extend_trail_[0];
				auto& v1 = extend_trail_[1];

				vec2s d = v1 - v0;
				vec2s n { -d.y, d.x };

				vec2s p0 = vec2s(v0)*CELL_SIZE + n*TRAIL_RADIUS;
				vec2s p1 = vec2s(v0)*CELL_SIZE - n*TRAIL_RADIUS;

				va.push_back({ p0.x, p0.y });
				va.push_back({ p1.x, p1.y });
			}

			// middle
			for (size_t i = 1; i < extend_trail_.size() - 1; i++) {
				auto& v0 = extend_trail_[i - 1];
				auto& v1 = extend_trail_[i];
				auto& v2 = extend_trail_[i + 1];

				vec2s ds = v1 - v0;
				vec2s ns { -ds.y, ds.x };

				vec2s de = v2 - v1;
				vec2s ne { -de.y, de.x };

				vec2s nm = ns + ne;

				int d = dot(ns, nm);

				vec2s p0 = vec2s(v1)*CELL_SIZE + nm*TRAIL_RADIUS/d;
				vec2s p1 = vec2s(v1)*CELL_SIZE - nm*TRAIL_RADIUS/d;

				va.push_back({ p0.x, p0.y });
				va.push_back({ p1.x, p1.y });
			}

			// last
			{
				auto& v0 = extend_trail_[extend_trail_.size() - 1];
				auto& v1 = extend_trail_[extend_trail_.size() - 2];

				vec2s d = v0 - v1;
				vec2s n { -d.y, d.x };

				vec2s p0 = vec2s(v0)*CELL_SIZE + n*TRAIL_RADIUS;
				vec2s p1 = vec2s(v0)*CELL_SIZE - n*TRAIL_RADIUS;

				va.push_back({ p0.x, p0.y });
				va.push_back({ p1.x, p1.y });
			}

			va.draw(GL_TRIANGLE_STRIP);
		}

		// last bit

		vec2s v0 = extend_trail_.back()*CELL_SIZE;
		vec2s v1 = get_position();

		short x0 = std::min(v0.x - TRAIL_RADIUS, v1.x - TRAIL_RADIUS);
		short x1 = std::max(v0.x + TRAIL_RADIUS, v1.x + TRAIL_RADIUS);

		short y0 = std::min(v0.y - TRAIL_RADIUS, v1.y - TRAIL_RADIUS);
		short y1 = std::max(v0.y + TRAIL_RADIUS, v1.y + TRAIL_RADIUS);

		(ggl::vertex_array_flat<GLshort, 2>
			{ { x0, y0 }, { x1, y0 },
			  { x0, y1 }, { x1, y1 } }).draw(GL_TRIANGLE_STRIP);
	}

	// head

	auto pos = get_position();

	glColor4f(1, 1, 1, 1);

	auto s = (state_ == state::EXTENDING || state_ == state::EXTENDING_IDLE ? sprites_core_ : sprites_shield_)[tic_%NUM_FRAMES];
	s->draw(pos.x, pos.y, ggl::sprite::horiz_align::CENTER, ggl::sprite::vert_align::CENTER);
}

const vec2i
player::get_position() const
{
	switch (state_) {
		case state::IDLE:
		case state::EXTENDING_IDLE:
			return pos_*CELL_SIZE;

		case state::SLIDING:
		case state::EXTENDING:
			{
			int d = CELL_SIZE*state_tics_/SLIDE_TICS;
			return pos_*CELL_SIZE + (next_pos_ - pos_)*d;
			}
	}
}

void
player::set_grid_position(const vec2i& p)
{
	pos_ = p;
}

vec2i
player::get_grid_position() const
{
	return pos_;
}

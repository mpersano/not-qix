#include <algorithm>
#include <cassert>

#include <ggl/sprite.h>
#include <ggl/resources.h>
#include <ggl/dpad_button.h>
#include <ggl/render.h>

#include "game.h"
#include "powerup.h"
#include "explosion.h"
#include "player.h"

namespace {

bool
dpad_button_pressed(unsigned dpad_state, ggl::dpad_button button)
{
	return dpad_state & (1u << button);
}

player::direction
dpad_direction(unsigned dpad_state)
{
	if (dpad_button_pressed(dpad_state, ggl::UP))
		return player::direction::UP;
	else if (dpad_button_pressed(dpad_state, ggl::DOWN))
		return player::direction::DOWN;
	else if (dpad_button_pressed(dpad_state, ggl::LEFT))
		return player::direction::LEFT;
	else if (dpad_button_pressed(dpad_state, ggl::RIGHT))
		return player::direction::RIGHT;
	else
		return player::direction::NONE;
}

const float PLAYER_RADIUS = 2;

const int SLIDE_TICS = 3;
const int DEATH_TICS = 20;
const int EXPLODE_SEGMENT_TICS = 2;

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
	lives_left_ = 2;
	respawn(pos);
}

void
player::respawn(const vec2i& pos)
{
	pos_ = pos;
	set_state(state::IDLE);

	respawn_event_.notify(lives_left_);
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

	auto slide_to = [&](const vec2i& where)
		{
			next_pos_ = where;
			set_state(state::SLIDING);
		};

	switch (dir) {
		case direction::UP:
			if (p[-1] != p[0])
				slide_to(pos_ + vec2i { 0, 1 });
			break;

		case direction::DOWN:
			if (p[-grid_cols - 1] != p[-grid_cols])
				slide_to(pos_ + vec2i { 0, -1 });
			break;

		case direction::LEFT:
			if (p[-grid_cols - 1] != p[-1])
				slide_to(pos_ + vec2i { -1, 0 });
			break;

		case direction::RIGHT:
			if (p[-grid_cols] != p[0])
				slide_to(next_pos_ = pos_ + vec2i { 1, 0 });
			break;
	}
}

bool
player::update(unsigned dpad_state)
{
	++state_tics_;

	switch (state_) {
		case state::IDLE:
			update_idle(dpad_state);
			break;

		case state::EXTENDING_IDLE:
			update_extending_idle(dpad_state);
			break;

		case state::SLIDING:
			update_sliding(dpad_state);
			break;

		case state::EXTENDING:
			update_extending(dpad_state);
			break;

		case state::EXPLODING:
			update_exploding(dpad_state);
			break;

		case state::DEATH:
			update_death(dpad_state);
			break;

		case state::DEAD:
			break;
	}

	return state_ != state::DEAD;
}

void
player::update_idle(unsigned dpad_state)
{
	direction dir = dpad_direction(dpad_state);

	if (dir != direction::NONE) {
		move_slide(dir);

		if (state_ != state::SLIDING && dpad_button_pressed(dpad_state, ggl::BUTTON1))
			move_extend(dir);
	}
}

void
player::update_sliding(unsigned dpad_state)
{
	if (state_tics_ >= SLIDE_TICS) {
		pos_ = next_pos_;
		set_state(state::IDLE);
	}
}

void
player::update_extending_idle(unsigned dpad_state)
{
	if (dpad_button_pressed(dpad_state, ggl::BUTTON1)) {
		move_extend(dpad_direction(dpad_state));
	} else {
		assert(!extend_trail_.empty());
		next_pos_ = extend_trail_.back();
		set_state(state::EXTENDING);
	}

	check_foe_collisions();
}

void
player::update_extending(unsigned dpad_state)
{
	if (state_tics_ >= SLIDE_TICS) {
		pos_ = next_pos_;

		if (!extend_trail_.empty() && extend_trail_.back() == pos_)
			extend_trail_.pop_back();

		if (extend_trail_.empty()) {
			set_state(state::IDLE);
			return;
		}

		const int grid_cols = game_.grid_cols;
		const int grid_rows = game_.grid_rows;
		auto *p = &game_.grid[pos_.y*grid_cols + pos_.x];

		if (p[0] || p[-1] || p[-grid_cols] || p[-grid_cols - 1]) {
			// filled region

			extend_trail_.push_back(pos_);

			game_.fill_grid(extend_trail_);

			extend_trail_.clear();
			set_state(state::IDLE);
			return;
		}

		state_ = state::EXTENDING_IDLE;
	}

	check_foe_collisions();
}

void
player::update_exploding(unsigned dpad_state)
{
	if (state_tics_%EXPLODE_SEGMENT_TICS == 0) {
		int index = state_tics_/EXPLODE_SEGMENT_TICS - 1;

		if (index == extend_trail_.size()) {
			set_state(state::DEATH);
		} else {
			if (index%2 == 0) {
				vec2f v0 = extend_trail_[index]*CELL_SIZE;
				vec2f v1 = index < extend_trail_.size() - 1 ? extend_trail_[index + 1]*CELL_SIZE : get_position();
				game_.add_effect(std::unique_ptr<effect>(new explosion(.5f*(v0 + v1), 1)));
			}
		}
	}
}

void
player::update_death(unsigned dpad_state)
{
	if (state_tics_ >= DEATH_TICS) {
		if (lives_left_) {
			--lives_left_;

			auto p = extend_trail_.front();
			extend_trail_.clear();
			respawn(p);
		} else {
			set_state(state::DEAD);
		}
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

						if (f->intersects(get_position(), PLAYER_RADIUS))
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

	// spawn powerups

	int num_powerups = 5;

	float da = 2.f*M_PI/num_powerups;
	float a = .5f*da;

	for (int i = 0; i < num_powerups; i++) {
		const float c = cosf(a);
		const float s = sinf(a);

		game_.add_entity(std::unique_ptr<entity>(new powerup(game_, get_position(), vec2f { c, s })));

		a += da;
	}

	game_.add_effect(std::unique_ptr<effect>(new explosion(get_position(), 2)));

	set_state(state::EXPLODING);

	death_event_.notify();
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
	switch (state_) {
		case state::EXTENDING:
		case state::EXTENDING_IDLE:
			draw_trail(0);
			// FALLTHRU

		case state::IDLE:
		case state::SLIDING:
			draw_head();
			break;

		case state::EXPLODING:
			draw_trail(state_tics_/EXPLODE_SEGMENT_TICS);
			break;
	}
}

void
player::draw_trail(int start_index) const
{
	static const int TRAIL_RADIUS = 1;

	if (extend_trail_.empty() || start_index == extend_trail_.size())
		return;

	assert(start_index < extend_trail_.size());

	trail_va_.clear();

	// first

	{
		auto& v0 = extend_trail_[start_index]*CELL_SIZE;
		auto& v1 = start_index + 1 < extend_trail_.size() ? extend_trail_[start_index + 1]*CELL_SIZE : get_position();

		vec2s d = normalized(v1 - v0);
		vec2s n { -d.y, d.x };

		vec2s p0 = vec2s(v0) + n*TRAIL_RADIUS;
		vec2s p1 = vec2s(v0) - n*TRAIL_RADIUS;

		trail_va_.push_back({ p0.x, p0.y });
		trail_va_.push_back({ p1.x, p1.y });
	}

	// middle

	for (size_t i = start_index + 1; i < extend_trail_.size(); i++) {
		auto& v0 = extend_trail_[i - 1]*CELL_SIZE;
		auto& v1 = extend_trail_[i]*CELL_SIZE;
		auto& v2 = i + 1 < extend_trail_.size() ? extend_trail_[i + 1]*CELL_SIZE : get_position();

		vec2s ds = normalized(v1 - v0);
		vec2s ns { -ds.y, ds.x };

		vec2s de = normalized(v2 - v1);
		vec2s ne { -de.y, de.x };

		vec2s nm = ns + ne;

		int d = dot(ns, nm);

		vec2s p0 = vec2s(v1) + nm*TRAIL_RADIUS/d;
		vec2s p1 = vec2s(v1) - nm*TRAIL_RADIUS/d;

		trail_va_.push_back({ p0.x, p0.y });
		trail_va_.push_back({ p1.x, p1.y });
	}

	// last

	{
		auto& v0 = get_position();
		auto& v1 = extend_trail_.back()*CELL_SIZE;

		vec2s d = normalized(v0 - v1);
		vec2s n { -d.y, d.x };

		vec2s p0 = vec2s(v0) + n*TRAIL_RADIUS;
		vec2s p1 = vec2s(v0) - n*TRAIL_RADIUS;

		trail_va_.push_back({ p0.x, p0.y });
		trail_va_.push_back({ p1.x, p1.y });
	}

	trail_va_.draw(GL_TRIANGLE_STRIP, ggl::rgba { 1, 1, 0, 1 });
}

void
player::draw_head() const
{
	auto pos = get_position();

	auto s = (state_ == state::EXTENDING || state_ == state::EXTENDING_IDLE ? sprites_core_ : sprites_shield_)[game_.tics%NUM_FRAMES];

	ggl::render::set_color(ggl::white);
	s->draw(0, vec2f { pos.x, pos.y });
}

const vec2i
player::get_position() const
{
	switch (state_) {
		case state::SLIDING:
		case state::EXTENDING:
			{
			int d = CELL_SIZE*state_tics_/SLIDE_TICS;
			return pos_*CELL_SIZE + (next_pos_ - pos_)*d;
			}

		default:
			return pos_*CELL_SIZE;
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

ggl::connectable_event<player::respawn_event_handler>&
player::get_respawn_event()
{
	return respawn_event_;
}

ggl::connectable_event<player::death_event_handler>&
player::get_death_event()
{
	return death_event_;
}

#include <algorithm>
#include <cassert>

#include "game.h"
#include "player.h"

player::player(game& g)
: game_ { g }
{ }

void
player::reset(const vec2i& pos)
{
	pos_ = pos;
	set_state(state::IDLE);
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
player::update()
{
	const int grid_cols = game_.grid_cols;
	const int grid_rows = game_.grid_rows;

	switch (state_) {
		case state::IDLE:
			break;

		case state::EXTENDING_IDLE:
			check_foe_collisions();
			break;

		case state::SLIDING:
			if (++state_tics_ >= SLIDE_TICS) {
				pos_ = next_pos_;
				state_ = state::IDLE;
			}
			break;

		case state::EXTENDING:
			if (++state_tics_ >= SLIDE_TICS) {
				pos_ = next_pos_;

				if (!extend_trail_.empty() && extend_trail_.back() == pos_)
					extend_trail_.pop_back();

				if (extend_trail_.empty()) {
					state_ = state::IDLE;
				} else {
					assert(pos_.x != 0 && pos_.x != grid_cols);
					assert(pos_.y != 0 && pos_.y != grid_rows);

					auto *p = &game_.grid[pos_.y*grid_cols + pos_.x];

					if (p[0] || p[-1] || p[-grid_cols] || p[-grid_cols - 1]) {
						extend_trail_.push_back(pos_);
						game_.fill_grid(extend_trail_);

						extend_trail_.clear();
						state_ = state::IDLE;
					} else {
						state_ = state::EXTENDING_IDLE;
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
		auto& foes = game_.foes;

		auto it = std::find_if(
				std::begin(foes),
				std::end(foes),
				[this](std::unique_ptr<foe>& f)
					{
						for (size_t i = 0; i < extend_trail_.size() - 1; i++) {
							const vec2i v0 = extend_trail_[i]*CELL_SIZE;
							const vec2i v1 = extend_trail_[i + 1]*CELL_SIZE;

							if (f->intersects(v0, v1))
								return true;
						}

						if (f->intersects(extend_trail_.back()*CELL_SIZE, get_position()))
								return true;

						return false;
					});

		if (it != std::end(foes)) {
			printf("collision!\n");

			pos_ = extend_trail_.front();
			extend_trail_.clear();
			state_ = state::IDLE;
		}
	}
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

	auto pos = vec2s(get_position());
	const short radius = 10;

	glColor4f(0, 1, 1, 1);

	(ggl::vertex_array_flat<GLshort, 2>
		{ { pos.x, pos.y },
		  { static_cast<short>(pos.x - radius), pos.y },
		  { pos.x, static_cast<short>(pos.y + radius) },
		  { static_cast<short>(pos.x + radius), pos.y },
		  { pos.x, static_cast<short>(pos.y - radius) },
		  { static_cast<short>(pos.x - radius), pos.y } }).draw(GL_TRIANGLE_FAN);
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

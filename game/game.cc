#include <queue>
#include <algorithm>
#include <cassert>

#include <ggl/gl.h>
#include <ggl/texture.h>
#include <ggl/vertex_array.h>
#include <ggl/resources.h>

#include "tween.h"
#include "level.h"
#include "boss.h"
#include "game.h"

//
//  p l a y e r
//

player::player(game& g)
: game_ { g }
{ }

void
player::reset()
{
	pos_ = vec2i { 0, 0 };
	set_state(state::IDLE);
}

void
player::move(direction dir)
{
	switch (state_) {
		case state::IDLE:
			move_slide(dir);

			if (state_ != state::SLIDING)
				move_extend(dir);
			break;

		case state::EXTENDING_IDLE:
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
			if (pos_.y < grid_rows) {
				if (pos_.x > 0 && pos_.x < grid_cols && !p[0] && !p[-1]) {
					extend_to(pos_ + vec2i { 0, 1 });
				}
			}
			break;

		case direction::DOWN:
			if (pos_.y > 0) {
				if (pos_.x > 0 && pos_.x < grid_cols && !p[-grid_cols] && !p[-grid_cols - 1]) {
					extend_to(pos_ + vec2i { 0, -1 });
				}
			}
			break;

		case direction::LEFT:
			if (pos_.x > 0) {
				if (pos_.y > 0 && pos_.y < grid_rows && !p[-grid_cols - 1] && !p[-1]) {
					extend_to(pos_ + vec2i { -1, 0 });
				}
			}
			break;

		case direction::RIGHT:
			if (pos_.x < grid_cols) {
				if (pos_.y > 0 && pos_.y < grid_rows && !p[-grid_cols] && !p[0]) {
					extend_to(pos_ + vec2i { 1, 0 });
				}
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
			if (pos_.y < grid_rows) {
				if ((pos_.x == 0 || p[-1]) != (pos_.x == grid_cols || p[0])) {
					next_pos_ = pos_ + vec2i { 0, 1 };
					set_state(state::SLIDING);
				}
			}
			break;

		case direction::DOWN:
			if (pos_.y > 0) {
				if ((pos_.x == 0 || p[-grid_cols - 1]) != (pos_.x == grid_cols || p[-grid_cols])) {
					next_pos_ = pos_ + vec2i { 0, -1 };
					set_state(state::SLIDING);
				}
			}
			break;

		case direction::LEFT:
			if (pos_.x > 0) {
				if ((pos_.y == 0 || p[-grid_cols - 1]) != (pos_.y == grid_rows || p[-1])) {
					next_pos_ = pos_ + vec2i { -1, 0 };
					set_state(state::SLIDING);
				}
			}
			break;

		case direction::RIGHT:
			if (pos_.x < grid_cols) {
				if ((pos_.y == 0 || p[-grid_cols]) != (pos_.y == grid_rows || p[0])) {
					next_pos_ = pos_ + vec2i { 1, 0 };
					set_state(state::SLIDING);
				}
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
					auto *p = &game_.grid[pos_.y*grid_cols + pos_.x];

					if (pos_.x == 0 ||
					    pos_.x == grid_cols ||
					    pos_.y == 0 ||
					    pos_.y == grid_rows ||
					    p[0] ||
					    p[-1] ||
					    p[-grid_cols] ||
					    p[-grid_cols - 1]) {
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


//
//  g a m e
//

static const float SCROLL_T = .2f;

game::game(int width, int height)
: viewport_width_ { width }
, viewport_height_ { height }
, player_ { *this }
{ }

void
game::reset(const level *l)
{
	cur_level_ = l;

	grid_rows = cur_level_->grid_rows;
	grid_cols = cur_level_->grid_cols;

	grid.resize(grid_rows*grid_cols);
	std::fill(std::begin(grid), std::end(grid), 0);

	offset_ = vec2i { 0, 0 };
	scrolling_ = false;
	cover_percent_ = 0u;

	initialize_vas();

	player_.reset();

	foes.push_back(std::unique_ptr<foe> { new boss { *this } });
}

void
game::draw() const
{
	glPushMatrix();

	auto offs = get_offset();
	glTranslatef(offs.x, offs.y, 0);

	draw_background();
	draw_border();

	for (auto& foe : foes)
		foe->draw();

	player_.draw();

	glPopMatrix();
}

vec2f
game::get_offset() const
{
	if (!scrolling_) {
		return offset_;
	} else {
		return quadratic_tween<vec2f>()(vec2f(offset_), vec2f(next_offset_), static_cast<float>(scroll_tics_)/SCROLL_TICS);
	}
}

void
game::initialize_vas()
{
	initialize_border();
	initialize_background_vas();
	initialize_border_va();
}

void
game::initialize_background_vas()
{
	auto& tex = cur_level_->fg_texture;

	const float du = static_cast<float>(tex->orig_width)/tex->width/grid_cols;
	const float dv = static_cast<float>(tex->orig_height)/tex->height/grid_rows;

	auto fill_spans = [&](ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2>& va, bool b)
		{
			va.clear();

			float v = 0;

			short y = 0;

			for (int i = 0; i < grid_rows; i++) {
				auto *row = &grid[i*grid_cols];
				auto *row_end = row + grid_cols;

				auto *span_start = row;

				const short yt = y + CELL_SIZE;

				while ((span_start = std::find(span_start, row_end, b)) != row_end) {
					auto *span_end = std::find(span_start, row_end, !b);

					auto s = std::distance(row, span_start);
					auto e = std::distance(row, span_end);

					short xs = s*CELL_SIZE;
					short xe = e*CELL_SIZE;

					auto us = s*du;
					auto ue = e*du;

					va.push_back({ xs, y, us, v });
					va.push_back({ xe, y, ue, v });
					va.push_back({ xe, yt, ue, v + dv });

					va.push_back({ xe, yt, ue, v + dv });
					va.push_back({ xs, yt, us, v + dv });
					va.push_back({ xs, y, us, v });

					span_start = span_end;
				}

				y += CELL_SIZE;
				v += dv;
			}
		};

	fill_spans(background_filled_va_, true);
	fill_spans(background_unfilled_va_, false);
}

void
game::initialize_border()
{
	border.clear();

	int coord = std::distance(std::begin(grid), std::find(std::begin(grid), std::end(grid), 0));
	assert(coord < grid_rows*grid_cols);

	vec2i start_pos { coord%grid_cols, coord/grid_cols };

	vec2i pos = start_pos;

	auto try_move = [&](const vec2i& d)
		{
			auto next = pos + d;

			if (border.empty() || dot(border.back() - pos, next - pos) <= 0) {
				// only push new verts if direction changed
				if (border.empty() || dot(border.back() - pos, next - pos) == 0) {
					border.push_back(pos);
				}

				pos = next;
				return true;
			}

			return false;
		};

	auto move_up = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.y < grid_rows &&
				(pos.x == 0 || p[-1]) != (pos.x == grid_cols || p[0]) &&
				try_move({ 0, 1 });
		};

	auto move_right = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.x < grid_cols &&
				(pos.y == 0 || p[-grid_cols]) != (pos.y == grid_rows || p[0]) &&
				try_move({ 1, 0 });
		};

	auto move_down = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.y > 0 &&
				(pos.x == 0 || p[-grid_cols - 1]) != (pos.x == grid_cols || p[-grid_cols]) &&
				try_move({ 0, -1 });
		};

	auto move_left = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.x > 0 &&
				(pos.y == 0 || p[-grid_cols - 1]) != (pos.y == grid_rows || p[-1]) &&
				try_move({ -1, 0 });
		};

	do {
		// tee-hee.
		move_up() || move_left() || move_down() || move_right() || (assert(0), false);
	} while (pos != start_pos);
}

void
game::initialize_border_va()
{
	static const int BORDER_RADIUS = 1;

	assert(!border.empty());

	border_va_.clear();

	const size_t border_size = border.size();

	for (size_t i = 0; i <= border_size; i++) {
		auto& v0 = border[(i + border_size - 1)%border_size];
		auto& v1 = border[i%border_size];
		auto& v2 = border[(i + 1)%border_size];

		vec2s ds = normalized(v1 - v0);
		vec2s ns { -ds.y, ds.x };

		vec2s de = normalized(v2 - v1);
		vec2s ne { -de.y, de.x };

		vec2s nm = ns + ne;

		short d = static_cast<float>(BORDER_RADIUS)/dot(ns, nm);

		vec2s p0 = vec2s(v1)*CELL_SIZE + nm*d;
		vec2s p1 = vec2s(v1)*CELL_SIZE - nm*d;

		border_va_.push_back({ p0.x, p0.y });
		border_va_.push_back({ p1.x, p1.y });
	}
}

void
game::draw_background() const
{
	glColor4f(1, 1, 1, 1);

	glEnable(GL_TEXTURE_2D);

	cur_level_->fg_texture->bind();
	background_filled_va_.draw(GL_TRIANGLES);

	cur_level_->bg_texture->bind();
	background_unfilled_va_.draw(GL_TRIANGLES);

	glDisable(GL_TEXTURE_2D);
}

void
game::draw_border() const
{
	glColor4f(1, 1, 1, 1);
	border_va_.draw(GL_TRIANGLE_STRIP);
}

void
game::move(direction dir)
{
	player_.move(dir);
}

void
game::update()
{
	// player

	player_.update();

	// foes

	auto it = std::begin(foes);

	while (it != std::end(foes)) {
		if (!(*it)->update())
			it = foes.erase(it);
		else
			++it;
	}

	// scrolling

	if (scrolling_) {
		if (++scroll_tics_ >= SCROLL_TICS) {
			offset_ = next_offset_;
			scrolling_ = false;
		}
	} else {
		static const int SCROLL_DIST = 100;

		vec2i pos = get_player_screen_position();

		if (pos.x < .2*viewport_width_) {
			if (offset_.x < 0) {
				next_offset_.x = std::min(0, offset_.x + SCROLL_DIST);
				next_offset_.y = offset_.y;

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		} else if (pos.x > .8*viewport_width_) {
			if (offset_.x + grid_cols*CELL_SIZE > viewport_width_) {
				next_offset_.x = std::max(viewport_width_ - grid_cols*CELL_SIZE, offset_.x - SCROLL_DIST);
				next_offset_.y = offset_.y;

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		}

		if (pos.y < .2*viewport_height_) {
			if (offset_.y < 0) {
				next_offset_.x = offset_.x;
				next_offset_.y = std::min(0, offset_.y + SCROLL_DIST);

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		} else if (pos.y > .8*viewport_height_) {
			if (offset_.y + grid_rows*CELL_SIZE > viewport_height_) {
				next_offset_.x = offset_.x;
				next_offset_.y = std::max(viewport_height_ - grid_rows*CELL_SIZE, offset_.y - SCROLL_DIST);

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		}
	}
}

void
game::fill_grid(const std::vector<vec2i>& contour)
{
	// flood fill from boss

	// forbidden transitions

	std::vector<std::pair<vec2i, vec2i>> transitions;

	for (size_t i = 0; i < contour.size() - 1; i++) {
		auto& u = contour[i];
		auto& v = contour[i + 1];

		vec2i p0, p1;

		if (v.y > u.y) {
			// up
			transitions.push_back(std::make_pair(u, u + vec2i { -1, 0 }));
		} else if (v.y < u.y) {
			// down
			transitions.push_back(std::make_pair(u + vec2i { -1, -1 }, u + vec2i { 0, -1 }));
		} else if (v.x < u.x) {
			// left
			transitions.push_back(std::make_pair(u + vec2i { -1, 0 }, u + vec2i { -1, -1 }));
		} else {
			// right
			transitions.push_back(std::make_pair(u, u + vec2i { 0, -1 }));
		}
	}

	assert(!foes.empty() && foes.front()->is_boss());
	auto *boss = static_cast<phys_foe *>(foes.front().get());

	vec2i pos = (vec2i(boss->get_position()) + vec2i { CELL_SIZE, CELL_SIZE }/2)/CELL_SIZE;

	std::queue<vec2i> queue;
	queue.push(pos);

	grid[pos.y*grid_cols + pos.x] = -1;

	while (!queue.empty()) {
		auto pos = queue.front();
		queue.pop();

		static const vec2i dirs[4] { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };

		for (auto& d : dirs) {
			auto next_pos = pos + d;

			if (next_pos.x < 0 || next_pos.x >= grid_cols || next_pos.y < 0 || next_pos.y >= grid_rows) {
				continue;
			}

			if (grid[next_pos.y*grid_cols + next_pos.x]) {
				continue;
			}

			auto it = std::find_if(
					std::begin(transitions),
					std::end(transitions),
					[=](const std::pair<vec2i, vec2i>& t)
						{ return (pos == t.first && next_pos == t.second) || (pos == t.second && next_pos == t.first); });

			if (it == std::end(transitions)) {
				grid[next_pos.y*grid_cols + next_pos.x] = -1;
				queue.push(next_pos);
			}
		}
	}

	for (auto& v : grid) {
		switch (v) {
			case -1: v = 0; break;
			case 0: v = 1; break;
			case 1: break;
			default: assert(0);
		}
	}

	// vertex arrays

	initialize_vas();

	// update cover percent

	unsigned cover = 0;

	for (size_t i = 0; i < grid_rows*grid_cols; i++) {
		if (grid[i]) {
			cover += cur_level_->silhouette[i];
		}
	}

	cover_percent_ = (static_cast<unsigned long long>(cover)*10000ull)/cur_level_->silhouette_pixels;
}

unsigned
game::get_cover_percent() const
{
	return cover_percent_;
}

vec2i
game::get_player_screen_position() const
{
	return player_.get_position() + offset_;
}

vec2i
game::get_player_world_position() const
{
	return player_.get_position();
}

void
game::add_foe(std::unique_ptr<foe> f)
{
	foes.push_back(std::move(f));
}

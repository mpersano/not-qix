#include <queue>
#include <algorithm>
#include <cassert>

#include <ggl/gl.h>
#include <ggl/texture.h>
#include <ggl/font.h>
#include <ggl/vertex_array.h>
#include <ggl/resources.h>

#include "tween.h"
#include "level.h"
#include "game.h"

//
//  p l a y e r
//

static const float SLIDE_T = .1f;

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
player::move(direction dir, bool button_pressed)
{
	switch (state_) {
		case state::IDLE:
			if (button_pressed) {
				move_extend(dir);
			} else {
				move_slide(dir);
			}
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

				if (extend_trail_.empty() || extend_trail_.back() != where) {
					extend_trail_.push_back(pos_);
				}

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
player::update(float dt)
{
	const int grid_cols = game_.grid_cols;
	const int grid_rows = game_.grid_rows;

	switch (state_) {
		case state::IDLE:
		case state::EXTENDING_IDLE:
			break;

		case state::SLIDING:
			if ((state_t_ += dt) >= SLIDE_T) {
				pos_ = next_pos_;
				state_ = state::IDLE;
			}
			break;

		case state::EXTENDING:
			if ((state_t_ += dt) >= SLIDE_T) {
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
			break;
	}
}

void
player::set_state(state next_state)
{
	state_ = next_state;
	state_t_ = 0;
}

void
player::draw() const
{
	glDisable(GL_TEXTURE_2D);

	// trail

	if (state_ == state::EXTENDING || state_ == state::EXTENDING_IDLE) {
		static const int TRAIL_RADIUS = 2;

		glColor4f(1, 0, 0, 1);

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
				auto& v2 = extend_trail_[i + 1]; // i < extend_trail_.size() - 1 ? extend_trail_[i + 1] : pos_;

				vec2s ds = v1 - v0;
				vec2s ns { -ds.y, ds.x };

				vec2s de = v2 - v1;
				vec2s ne { -de.y, de.x };

				vec2s nm = ns + ne;

				short d = static_cast<float>(TRAIL_RADIUS)/dot(ns, nm);

				vec2s p0 = vec2s(v1)*CELL_SIZE + nm*d;
				vec2s p1 = vec2s(v1)*CELL_SIZE - nm*d;

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

	const short x0 = pos.x - PLAYER_RADIUS;
	const short x1 = pos.x + PLAYER_RADIUS;
	const short y0 = pos.y - PLAYER_RADIUS;
	const short y1 = pos.y + PLAYER_RADIUS;

	glColor4f(0, 1, 0, 1);

	(ggl::vertex_array_flat<GLshort, 2>
		{ { x0, y0 }, { x1, y0 },
		  { x0, y1 }, { x1, y1 } }).draw(GL_TRIANGLE_STRIP);
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
			int d = CELL_SIZE*state_t_/SLIDE_T;
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
, font_ { ggl::res::get_font("fonts/tiny") }
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
	cover_percentage_ = 0u;

	initialize_vas();

	player_.reset();
}

void
game::draw() const
{
	auto offs = get_offset();
	glTranslatef(offs.x, offs.y, 0);

	draw_background();
	draw_border();

	player_.draw();

	glColor4f(1, 1, 1, 1);

#if 0
	glPushMatrix();
	glTranslatef(50, 50, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	font_->render(L"hello, world");
	glDisable(GL_BLEND);

	glPopMatrix();
#endif
}

vec2f
game::get_offset() const
{
	if (!scrolling_) {
		return offset_;
	} else {
		return quadratic_tween<vec2f>()(vec2f(offset_), vec2f(next_offset_), scroll_t_/SCROLL_T);
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
	border_.clear();

	int coord = std::distance(std::begin(grid), std::find(std::begin(grid), std::end(grid), 0));
	assert(coord < grid_rows*grid_cols);

	vec2i start_pos { coord%grid_cols, coord/grid_cols };

	vec2i pos = start_pos;

	auto try_move = [&](const vec2i& d)
		{
			auto next = pos + d;
			if (border_.empty() || border_.back() != next) {
				// TODO: only add pos to border_ if direction changed
				border_.push_back(pos);
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

	printf("%d verts\n", border_.size());
}

void
game::initialize_border_va()
{
	static const int BORDER_RADIUS = 2;

	assert(!border_.empty());

	border_va_.clear();

	const size_t border_size = border_.size();

	for (size_t i = 0; i <= border_size; i++) {
		auto& v0 = border_[(i + border_size - 1)%border_size];
		auto& v1 = border_[i%border_size];
		auto& v2 = border_[(i + 1)%border_size];

		vec2s ds = v1 - v0;
		vec2s ns { -ds.y, ds.x };

		vec2s de = v2 - v1;
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
	glColor4f(1, 1, 0, 1);
#if 1
	border_va_.draw(GL_TRIANGLE_STRIP);
#else
	glBegin(GL_LINE_LOOP);
	for (auto& v : border_)
		glVertex2i(v.x*CELL_SIZE, v.y*CELL_SIZE);
	glEnd();
#endif
}

void
game::move(direction dir, bool button_pressed)
{
	player_.move(dir, button_pressed);
}

void
game::update(float dt)
{
	player_.update(dt);

	if (scrolling_) {
		if ((scroll_t_ += dt) >= SCROLL_T) {
			offset_ = next_offset_;
			scrolling_ = false;
		}
	} else {
		static const int SCROLL_DIST = 100;

		vec2i pos = player_.get_position() + offset_;

		if (pos.x < .2*viewport_width_) {
			if (offset_.x < 0) {
				next_offset_.x = std::min(0, offset_.x + SCROLL_DIST);
				next_offset_.y = offset_.y;

				scroll_t_ = 0;
				scrolling_ = true;
			}
		} else if (pos.x > .8*viewport_width_) {
			if (offset_.x + grid_cols*CELL_SIZE > viewport_width_) {
				next_offset_.x = std::max(viewport_width_ - grid_cols*CELL_SIZE, offset_.x - SCROLL_DIST);
				next_offset_.y = offset_.y;

				scroll_t_ = 0;
				scrolling_ = true;
			}
		}

		if (pos.y < .2*viewport_height_) {
			if (offset_.y < 0) {
				next_offset_.x = offset_.x;
				next_offset_.y = std::min(0, offset_.y + SCROLL_DIST);

				scroll_t_ = 0;
				scrolling_ = true;
			}
		} else if (pos.y > .8*viewport_height_) {
			if (offset_.y + grid_rows*CELL_SIZE > viewport_height_) {
				next_offset_.x = offset_.x;
				next_offset_.y = std::max(viewport_height_ - grid_rows*CELL_SIZE, offset_.y - SCROLL_DIST);

				scroll_t_ = 0;
				scrolling_ = true;
			}
		}
	}
}

void
game::fill_grid(const std::vector<vec2i>& contour)
{
	// flood fill

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

	auto fill = [&](std::vector<int>& grid, int coord)
		{
			std::queue<vec2i> queue;
			queue.push(vec2i { coord%grid_cols, coord/grid_cols });

			grid[coord] = 1;

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

					bool allow = true;
					for (auto& t : transitions) {
						if ((pos == t.first && next_pos == t.second) || (pos == t.second && next_pos == t.first)) {
							allow = false;
							break;
						}
					}

					if (allow) {
						grid[next_pos.y*grid_cols + next_pos.x] = 1;
						queue.push(next_pos);
					}
				}
			}
		};

	auto next_grid_0 = grid;

	{
	int coord = std::distance(std::begin(grid), std::find(std::begin(grid), std::end(grid), 0));
	fill(next_grid_0, coord);
	}

	auto next_grid_1 = grid;

	{
	int coord = std::distance(std::begin(next_grid_0), std::find(std::begin(next_grid_0), std::end(next_grid_0), 0));
	fill(next_grid_1, coord);
	}

	int count0 = std::accumulate(std::begin(next_grid_0), std::end(next_grid_0), 0);
	int count1 = std::accumulate(std::begin(next_grid_1), std::end(next_grid_1), 0);

	if (count0 < count1) {
		std::copy(std::begin(next_grid_0), std::end(next_grid_0), std::begin(grid));
	} else {
		std::copy(std::begin(next_grid_1), std::end(next_grid_1), std::begin(grid));
	}

	// vertex arrays

	initialize_vas();

	// update cover percentage

	unsigned cover = 0;

	for (size_t i = 0; i < grid_rows*grid_cols; i++) {
		if (grid[i]) {
			cover += cur_level_->silhouette[i];
		}
	}

	cover_percentage_ = (cover*10000)/cur_level_->silhouette_pixels; // XXX: will probably overflow on large images
}

unsigned
game::get_cover_percentage() const
{
	return cover_percentage_;
}

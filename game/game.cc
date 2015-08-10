#include <queue>
#include <algorithm>

#include <GL/glew.h>

#include <ggl/font.h>
#include <ggl/vertex_array.h>
#include <ggl/resources.h>

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
	auto *p = &game_.grid[pos_.y*GRID_COLS + pos_.x];

	auto slide_to = [&](const vec2i& where)
		{
			auto it = std::find(std::begin(extend_trail_), std::end(extend_trail_), where);

			if (it == extend_trail_.end() || it + 1 == extend_trail_.end()) {
				next_pos_ = where;

				if (!(!extend_trail_.empty() && extend_trail_.back() == where)) {
					extend_trail_.push_back(pos_);
				}

				set_state(state::EXTENDING);
			}
		};

	switch (dir) {
		case direction::UP:
			if (pos_.y < GRID_ROWS) {
				if (pos_.x > 0 && pos_.x < GRID_COLS && !p[0] && !p[-1]) {
					slide_to(pos_ + vec2i { 0, 1 });
				}
			}
			break;

		case direction::DOWN:
			if (pos_.y > 0) {
				if (pos_.x > 0 && pos_.x < GRID_COLS && !p[-GRID_COLS] && !p[-GRID_COLS - 1]) {
					slide_to(pos_ + vec2i { 0, -1 });
				}
			}
			break;

		case direction::LEFT:
			if (pos_.x > 0) {
				if (pos_.y > 0 && pos_.y < GRID_ROWS && !p[-GRID_COLS - 1] && !p[-1]) {
					slide_to(pos_ + vec2i { -1, 0 });
				}
			}
			break;

		case direction::RIGHT:
			if (pos_.x < GRID_COLS) {
				if (pos_.y > 0 && pos_.y < GRID_ROWS && !p[-GRID_COLS] && !p[0]) {
					slide_to(pos_ + vec2i { 1, 0 });
				}
			}
			break;
	}
}

void
player::move_slide(direction dir)
{
	auto *p = &game_.grid[pos_.y*GRID_COLS + pos_.x];

	switch (dir) {
		case direction::UP:
			if (pos_.y < GRID_ROWS) {
				if ((pos_.x == 0 || p[-1]) != (pos_.x == GRID_COLS || p[0])) {
					next_pos_ = pos_ + vec2i { 0, 1 };
					set_state(state::SLIDING);
				}
			}
			break;

		case direction::DOWN:
			if (pos_.y > 0) {
				if ((pos_.x == 0 || p[-GRID_COLS - 1]) != (pos_.x == GRID_COLS || p[-GRID_COLS])) {
					next_pos_ = pos_ + vec2i { 0, -1 };
					set_state(state::SLIDING);
				}
			}
			break;

		case direction::LEFT:
			if (pos_.x > 0) {
				if ((pos_.y == 0 || p[-GRID_COLS - 1]) != (pos_.y == GRID_ROWS || p[-1])) {
					next_pos_ = pos_ + vec2i { -1, 0 };
					set_state(state::SLIDING);
				}
			}
			break;

		case direction::RIGHT:
			if (pos_.x < GRID_COLS) {
				if ((pos_.y == 0 || p[-GRID_COLS]) != (pos_.y == GRID_ROWS || p[0])) {
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

				if (!extend_trail_.empty() && extend_trail_.back() == pos_) {
					extend_trail_.pop_back();
				}

				if (extend_trail_.empty()) {
					state_ = state::IDLE;
				} else {
					auto *p = &game_.grid[pos_.y*GRID_COLS + pos_.x];

					if (pos_.x == 0 ||
					    pos_.x == GRID_COLS ||
					    pos_.y == 0 ||
					    pos_.y == GRID_ROWS ||
					    p[0] ||
					    p[-1] ||
					    p[-GRID_COLS] ||
					    p[-GRID_COLS - 1]) {
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
	glColor4f(0, 1, 0, 1);

	auto pos = get_position();

	glBegin(GL_QUADS);
	glVertex2i(pos.x - 5, pos.y - 5);
	glVertex2i(pos.x + 5, pos.y - 5);
	glVertex2i(pos.x + 5, pos.y + 5);
	glVertex2i(pos.x - 5, pos.y + 5);
	glEnd();

	if (state_ == state::EXTENDING || state_ == state::EXTENDING_IDLE) {
		static const int TRAIL_RADIUS = 2;

		static auto draw_segment = [](const vec2i& u, const vec2i& v)
			{
				const int x0 = std::min(u.x - TRAIL_RADIUS, v.x - TRAIL_RADIUS);
				const int x1 = std::max(u.x + TRAIL_RADIUS, v.x + TRAIL_RADIUS);
				const int y0 = std::min(u.y - TRAIL_RADIUS, v.y - TRAIL_RADIUS);
				const int y1 = std::max(u.y + TRAIL_RADIUS, v.y + TRAIL_RADIUS);

				glVertex2i(x0, y0);
				glVertex2i(x1, y0);
				glVertex2i(x1, y1);
				glVertex2i(x0, y1);
			};

		glBegin(GL_QUADS);

		for (size_t i = 0; i < extend_trail_.size() - 1; i++) {
			auto& u = extend_trail_[i]*game_.cell_size;
			auto& v = extend_trail_[i + 1]*game_.cell_size;

			draw_segment(u, v);
		}

		auto& v = extend_trail_.back()*game_.cell_size;
		draw_segment(v, pos);

		glEnd();
	}
}

const vec2i
player::get_position() const
{
	switch (state_) {
		case state::IDLE:
		case state::EXTENDING_IDLE:
			return pos_*game_.cell_size;

		case state::SLIDING:
		case state::EXTENDING:
			{
			int d = game_.cell_size*state_t_/SLIDE_T;
			return pos_*game_.cell_size + (next_pos_ - pos_)*d;
			}
	}
}


//
//  g a m e
//

game::game(int width, int height)
: cell_size { std::min(width/GRID_COLS, height/GRID_ROWS) }
, base_x_ { (width - cell_size*GRID_COLS)/2 }
, base_y_ { (height - cell_size*GRID_ROWS)/2 }
, player_ { *this }
, font_ { ggl::res::get_font("data/fonts/tiny") }
{ }

void
game::reset(const level *l)
{
	cur_level_ = l;

	std::fill(std::begin(grid), std::end(grid), false);
	initialize_vas();

	player_.reset();

	elapsed_t_ = 0;
}

void
game::draw() const
{
	glPushMatrix();
	glTranslatef(base_x_, base_y_, 0);

	draw_background();
	draw_border();

	player_.draw();

	glColor4f(1, 1, 1, 1);

	glPushMatrix();
	glTranslatef(50, 50, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	font_->render(L"hello, world");
	glDisable(GL_BLEND);

	glPopMatrix();

	glPopMatrix();
}

void
game::initialize_vas()
{
	initialize_background_vas();
	initialize_border_va();
}

void
game::initialize_background_vas()
{
	auto& tex = cur_level_->background_texture;

	const float du = static_cast<float>(tex->orig_width)/tex->width/GRID_COLS;
	const float dv = static_cast<float>(tex->orig_height)/tex->height/GRID_ROWS;

	auto fill_spans = [&](ggl::vertex_array_texcoord<GLint, 2, GLfloat, 2>& va, bool b)
		{
			va.clear();

			int y = 0;
			float v = 0;

			for (int i = 0; i < GRID_ROWS; i++) {
				auto *row = &grid[i*GRID_COLS];
				auto *row_end = row + GRID_COLS;

				auto *span_start = row;

				while ((span_start = std::find(span_start, row_end, b)) != row_end) {
					auto *span_end = std::find(span_start, row_end, !b);

					auto s = std::distance(row, span_start);
					auto e = std::distance(row, span_end);

					auto xs = static_cast<int>(s*cell_size);
					auto xe = static_cast<int>(e*cell_size);

					auto us = s*du;
					auto ue = e*du;

					va.push_back({ xs, y, us, v });
					va.push_back({ xe, y, ue, v });
					va.push_back({ xe, y + cell_size, ue, v + dv });
					va.push_back({ xs, y + cell_size, us, v + dv });

					span_start = span_end;
				}

				y += cell_size;
				v += dv;
			}
		};

	fill_spans(background_filled_va_, true);
	fill_spans(background_unfilled_va_, false);
}

void
game::initialize_border_va()
{
	border_va_.clear();

	static const int BORDER_RADIUS = 1;

	int y = 0;

	for (int i = 0; i < GRID_ROWS; i++) {
		int x = 0;

		for (int j = 0; j < GRID_COLS; j++) {
			auto *p = &grid[i*GRID_COLS + j];

			if (!*p) {
				const int x0 = x - BORDER_RADIUS;
				const int x1 = x + BORDER_RADIUS;

				const int x2 = x + cell_size - BORDER_RADIUS;
				const int x3 = x + cell_size + BORDER_RADIUS;

				const int y0 = y - BORDER_RADIUS;
				const int y1 = y + BORDER_RADIUS;

				const int y2 = y + cell_size - BORDER_RADIUS;
				const int y3 = y + cell_size + BORDER_RADIUS;

				// top
				if (i == GRID_ROWS - 1 || p[GRID_COLS]) {
					border_va_.push_back({ x0, y2 });
					border_va_.push_back({ x3, y2 });
					border_va_.push_back({ x3, y3 });
					border_va_.push_back({ x0, y3 });
				}

				// down
				if (i == 0 || p[-GRID_COLS]) {
					border_va_.push_back({ x0, y0 });
					border_va_.push_back({ x3, y0 });
					border_va_.push_back({ x3, y1 });
					border_va_.push_back({ x0, y1 });
				}

				// left
				if (j == 0 || p[-1]) {
					border_va_.push_back({ x0, y0 });
					border_va_.push_back({ x0, y3 });
					border_va_.push_back({ x1, y3 });
					border_va_.push_back({ x1, y0 });
				}

				// right
				if (j == GRID_COLS - 1 || p[1]) {
					border_va_.push_back({ x2, y0 });
					border_va_.push_back({ x2, y3 });
					border_va_.push_back({ x3, y3 });
					border_va_.push_back({ x3, y0 });
				}
			}

			x += cell_size;
		}

		y += cell_size;
	}
}

void
game::draw_background() const
{
	glColor4f(1, 1, 1, 1);

	glEnable(GL_TEXTURE_2D);

	cur_level_->background_texture->bind();
	background_filled_va_.draw(GL_QUADS);

	cur_level_->mask_texture->bind();
	background_unfilled_va_.draw(GL_QUADS);

	glDisable(GL_TEXTURE_2D);
}

void
game::draw_border() const
{
	glColor4f(1, 1, 0, 1);

	border_va_.draw(GL_QUADS);
}

void
game::move(direction dir, bool button_pressed)
{
	player_.move(dir, button_pressed);
}

void
game::update(float dt)
{
	elapsed_t_ += dt;

	player_.update(dt);
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

	auto fill = [&](bool *grid, int coord)
		{
			std::queue<vec2i> queue;
			queue.push(vec2i { coord%GRID_COLS, coord/GRID_COLS });

			grid[coord] = true;

			while (!queue.empty()) {
				auto pos = queue.front();
				queue.pop();

				static const vec2i dirs[4] { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };

				for (auto& d : dirs) {
					auto next_pos = pos + d;

					if (next_pos.x < 0 || next_pos.x >= GRID_COLS || next_pos.y < 0 || next_pos.y >= GRID_ROWS) {
						continue;
					}

					if (grid[next_pos.y*GRID_COLS + next_pos.x]) {
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
						grid[next_pos.y*GRID_COLS + next_pos.x] = true;
						queue.push(next_pos);
					}
				}
			}
		};

	static bool next_grid_0[GRID_ROWS*GRID_COLS];
	std::copy(std::begin(grid), std::end(grid), next_grid_0);

	{
	int coord = std::distance(std::begin(grid), std::find(std::begin(grid), std::end(grid), false));
	fill(next_grid_0, coord);
	}

	static bool next_grid_1[GRID_ROWS*GRID_COLS];
	std::copy(std::begin(grid), std::end(grid), next_grid_1);

	{
	int coord = std::distance(std::begin(next_grid_0), std::find(std::begin(next_grid_0), std::end(next_grid_0), false));
	fill(next_grid_1, coord);
	}

	int count0 = std::accumulate(std::begin(next_grid_0), std::end(next_grid_0), 0);
	int count1 = std::accumulate(std::begin(next_grid_1), std::end(next_grid_1), 0);

	if (count0 < count1) {
		std::copy(std::begin(next_grid_0), std::end(next_grid_0), grid);
	} else {
		std::copy(std::begin(next_grid_1), std::end(next_grid_1), grid);
	}

	// vertex arrays

	initialize_vas();

	// update cover percentage

	int cover = 0;

	for (size_t i = 0; i < GRID_ROWS*GRID_COLS; i++) {
		if (grid[i]) {
			cover += cur_level_->silhouette[i];
		}
	}

	printf("%d/%d cover: %.2f %%\n",
		cover, cur_level_->silhouette_pixels,
		static_cast<float>(cover*100)/cur_level_->silhouette_pixels);
}

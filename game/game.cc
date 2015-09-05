#include <queue>
#include <algorithm>
#include <cassert>

#include <ggl/gl.h>
#include <ggl/dpad_button.h>
#include <ggl/texture.h>
#include <ggl/vertex_array.h>
#include <ggl/resources.h>

#include "util.h"
#include "tween.h"
#include "level.h"
#include "boss.h"
#include "game.h"

namespace {

const int BORDER_RADIUS = 1;

}

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

	initialize_background();

	state_ = state::SELECTING_START_AREA;
	reset_start_area();

	prev_dpad_state_ = 0;
}

void
game::draw() const
{
	glPushMatrix();

	auto offs = get_offset();
	glTranslatef(offs.x, offs.y, 0);

	draw_background();

	switch (state_) {
		case state::SELECTING_START_AREA:
			draw_selecting_start_area();
			break;

		case state::PLAYING:
			draw_playing();
			break;
	}

	glPopMatrix();
}

void
game::draw_selecting_start_area() const
{
	short x0 = start_area_.first.x*CELL_SIZE;
	short x1 = start_area_.second.x*CELL_SIZE;

	short y0 = start_area_.first.y*CELL_SIZE;
	short y1 = start_area_.second.y*CELL_SIZE;

	glColor4f(1, 1, 1, .5);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	(ggl::vertex_array_flat<GLshort, 2>
		{ { x0, y0 }, { x1, y0 },
		  { x0, y1 }, { x1, y1 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_BLEND);

	glColor4f(1, 1, 1, 1);

	short x00 = x0 - BORDER_RADIUS;
	short x01 = x0 + BORDER_RADIUS;

	short x10 = x1 - BORDER_RADIUS;
	short x11 = x1 + BORDER_RADIUS;

	short y00 = y0 - BORDER_RADIUS;
	short y01 = y0 + BORDER_RADIUS;

	short y10 = y1 - BORDER_RADIUS;
	short y11 = y1 + BORDER_RADIUS;

	(ggl::vertex_array_flat<GLshort, 2>
		{ { x00, y00 }, { x01, y01 },
		  { x11, y00 }, { x10, y01 },
		  { x11, y11 }, { x10, y10 },
		  { x00, y11 }, { x01, y10 },
		  { x00, y00 }, { x01, y01 } }).draw(GL_TRIANGLE_STRIP);
}

void
game::draw_playing() const
{
	draw_border();

	for (auto& foe : foes)
		foe->draw();

	player_.draw();
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
game::initialize_background()
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
	//
	//  find border verts
	//

	border.clear();

	vec2i start_pos = player_.get_grid_position();

	vec2i pos = start_pos, prev_pos = start_pos;

	auto try_move = [&](const vec2i& d)
		{
			auto next_pos = pos + d;

			if (prev_pos == pos || dot(prev_pos - pos, next_pos - pos) <= 0) {
				// only push new verts if direction changed
				if (prev_pos != pos && dot(prev_pos - pos, next_pos - pos) == 0)
					border.push_back(pos);

				prev_pos = pos;
				pos = next_pos;
				return true;
			}

			return false;
		};

	auto move_up = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.y < grid_rows &&
				p[-1] != p[0] &&
				try_move({ 0, 1 });
		};

	auto move_right = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.x < grid_cols &&
				p[-grid_cols] != p[0] &&
				try_move({ 1, 0 });
		};

	auto move_down = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.y > 0 &&
				p[-grid_cols - 1] != p[-grid_cols] &&
				try_move({ 0, -1 });
		};

	auto move_left = [&]()
		{
			auto *p = &grid[pos.y*grid_cols + pos.x];
			return pos.x > 0 &&
				p[-grid_cols - 1] != p[-1] &&
				try_move({ -1, 0 });
		};

	do {
		// tee-hee.
		move_up() || move_left() || move_down() || move_right() || (assert(0), false);
	} while (border.empty() || pos != border.front());

	printf("%lu verts\n", border.size());

	//
	// border vertex array
	//

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
game::start_playing()
{
	for (int r = start_area_.first.y; r < start_area_.second.y; r++) {
		auto *row = &grid[r*grid_cols];
		std::fill(row + start_area_.first.x, row + start_area_.second.x, 1);
	}

	player_.reset(start_area_.first); // needs to be called before initialize_border

	initialize_border();
	initialize_background();

	update_cover_percent();

	const vec2f boss_pos { 30, 30 }; // XXX: pick initial boss position

	foes.push_back(std::unique_ptr<foe> { new boss { *this, boss_pos } });

	state_ = state::PLAYING;
}

void
game::update(unsigned dpad_state)
{
	switch (state_) {
		case state::SELECTING_START_AREA:
			update_selecting_start_area(dpad_state);
			break;

		case state::PLAYING:
			update_playing(dpad_state);
			break;
	}

	prev_dpad_state_ = dpad_state;
}

void
game::update_selecting_start_area(unsigned dpad_state)
{
	if ((dpad_state & (1u << ggl::BUTTON1)) && (prev_dpad_state_ & (1u << ggl::BUTTON1))) {
		start_playing();
	} else {
		if (--start_area_tics_ == 0)
			reset_start_area();
	}
}

void
game::update_playing(unsigned dpad_state)
{
	// player

	auto dpad_button_pressed = [=](ggl::dpad_button button)
		{
			return dpad_state & (1u << button);
		};

	bool button = dpad_button_pressed(ggl::BUTTON1);

	if (dpad_button_pressed(ggl::UP))
		player_.move(direction::UP, button);

	if (dpad_button_pressed(ggl::DOWN))
		player_.move(direction::DOWN, button);

	if (dpad_button_pressed(ggl::LEFT))
		player_.move(direction::LEFT, button);

	if (dpad_button_pressed(ggl::RIGHT))
		player_.move(direction::RIGHT, button);

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

	// update vertex arrays

	initialize_border();
	initialize_background();

	// update cover percent

	update_cover_percent();
}

void
game::update_cover_percent()
{
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

void
game::set_player_grid_position(const vec2i& p)
{
	player_.set_grid_position(p);
}

void
game::reset_start_area()
{
	static const int BORDER = 8;

	const vec2i v0 = offset_/CELL_SIZE;
	const vec2i v1 { std::min(grid_cols, v0.x + viewport_width_/CELL_SIZE), std::min(grid_rows, v0.y + viewport_height_/CELL_SIZE) };

	vec2i from { rand(v0.x + BORDER, v1.x - BORDER), rand(v0.y + BORDER, v1.y - BORDER) };
	vec2i to { rand(from.x + 1, v1.x - BORDER + 1), rand(from.y + 1, v1.y - BORDER + 1) };

	start_area_ = std::make_pair(from, to);

	start_area_tics_ = 5;
}

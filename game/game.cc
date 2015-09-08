#include <queue>
#include <algorithm>
#include <cassert>

#include <ggl/gl.h>
#include <ggl/dpad_button.h>
#include <ggl/texture.h>
#include <ggl/font.h>
#include <ggl/vertex_array.h>
#include <ggl/resources.h>

#include "util.h"
#include "tween.h"
#include "quad.h"
#include "action.h"
#include "level.h"
#include "boss.h"
#include "game.h"

namespace {

const int BORDER_RADIUS = 1;

class level_intro_state : public game_state
{
public:
	level_intro_state(game& g);

	void draw() const override;
	void update(unsigned dpad_state) override;

private:
	void init_portrait();
	void init_stage_text();

	image_quad portrait_;
	vec2f portrait_pos_;

	text_quad stage_text_;
	vec2f stage_text_pos_;

	text_quad name_text_;
	vec2f name_text_pos_;

	std::unique_ptr<abstract_action> action_;
};

class offset_select_state : public game_state
{
public:
	offset_select_state(game& g);

	void draw() const override;
	void update(unsigned dpad_state) override;

private:
	static const int SCROLL_TICS = 30;

	int scroll_tics_;
	bool scroll_dir_;
	unsigned prev_dpad_state_;
};

class start_select_state : public game_state
{
public:
	start_select_state(game& g);

	void draw() const override;
	void update(unsigned dpad_state) override;

private:
	void on_button_down();
	void reset_start_area();

	std::pair<vec2i, vec2i> start_area_;

	int change_tics_;
	unsigned prev_dpad_state_;
};

class playing_state : public game_state
{
public:
	playing_state(game& g);

	void draw() const override;
	void update(unsigned dpad_state) override;

private:
	bool scrolling_;
	int scroll_tics_;
	vec2i prev_offset_, next_offset_; // when scrolling
};

bool
button_pressed(unsigned dpad_state, ggl::dpad_button button)
{
	return dpad_state & (1u << button);
}

//
//  l e v e l _ i n t r o _ s t a t e
//

level_intro_state::level_intro_state(game& g)
: game_state { g }
, portrait_(game_.cur_level->portrait_texture)
, stage_text_(ggl::res::get_font("fonts/small.spr"), L"stage 1")
, name_text_(ggl::res::get_font("fonts/tiny.spr"), L"pearl girl")
{
	const auto w = game_.viewport_width;
	const auto h = game_.viewport_height;

	action_.reset(
		(new sequential_action_group)->add(
			(new parallel_action_group)->add(
				new property_change_action<quadratic_tween<vec2f>>(
					portrait_pos_,
					vec2f { w + .5f*portrait_.get_width(), .5f*h },
					vec2f { .5f*w, .5f*h },
					30))->add(
				new property_change_action<quadratic_tween<vec2f>>(
					stage_text_pos_,
					vec2f { -.5f*stage_text_.get_width(), .5f*h + 60 },
					vec2f { .5f*w, .5f*h + 60 },
					30))->add(
				new property_change_action<quadratic_tween<vec2f>>(
					name_text_pos_,
					vec2f { -.5f*name_text_.get_width(), .5f*h - 60 },
					vec2f { .5f*w, .5f*h - 60 },
					30)))->add(
			new delay_action(60)));

	action_->set_properties();
}

void
level_intro_state::draw() const
{
	glColor4f(1, 1, 1, 1);

	glEnable(GL_TEXTURE_2D);

	// portrait

	glPushMatrix();
	glTranslatef(portrait_pos_.x, portrait_pos_.y, 0);
	portrait_.draw();
	glPopMatrix();

	// stage text

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glTranslatef(stage_text_pos_.x, stage_text_pos_.y, 0);
	stage_text_.draw();
	glPopMatrix();

	// name text

	glPushMatrix();
	glTranslatef(name_text_pos_.x, name_text_pos_.y, 0.f);
	name_text_.draw();
	glPopMatrix();

	glDisable(GL_BLEND);

	glDisable(GL_TEXTURE_2D);
}

void
level_intro_state::update(unsigned dpad_state)
{
	action_->step();

	if (action_->done()) {
		game_.set_state(std::unique_ptr<game_state>(new offset_select_state { game_ }));
	}
}

//
//  o f f s e t _ s e l e c t _ s t a t e
//

offset_select_state::offset_select_state(game& g)
: game_state { g }
, scroll_tics_ { SCROLL_TICS }
, scroll_dir_ { false }
, prev_dpad_state_ { ~0u }
{ }

void
offset_select_state::draw() const
{ }

void
offset_select_state::update(unsigned dpad_state)
{
	if (prev_dpad_state_ != ~0u &&
	  button_pressed(dpad_state, ggl::BUTTON1) && !button_pressed(prev_dpad_state_, ggl::BUTTON1)) {
		game_.set_state(std::unique_ptr<game_state>(new start_select_state { game_ }));
		return;
	}

	prev_dpad_state_ = dpad_state;

	if (--scroll_tics_ == 0) {
		scroll_tics_ = SCROLL_TICS;
		scroll_dir_ = !scroll_dir_;
	}

	vec2f from, to;

	if (scroll_dir_) {
		from = vec2f { 0, 0 };
		to = vec2f { 0, -game_.grid_rows*CELL_SIZE + game_.viewport_height };
	} else {
		from = vec2f { 0, -game_.grid_rows*CELL_SIZE + game_.viewport_height };
		to = vec2f { 0, 0 };
	}

	game_.offset = quadratic_tween<vec2f>()(from, to, static_cast<float>(scroll_tics_)/SCROLL_TICS);
}

//
//  s t a r t _ s e l e c t _ s t a t e
//

start_select_state::start_select_state(game& g)
: game_state { g }
, prev_dpad_state_ { ~0u }
{
	reset_start_area();
}

void
start_select_state::draw() const
{
	short x0 = start_area_.first.x*CELL_SIZE;
	short x1 = start_area_.second.x*CELL_SIZE;

	short y0 = start_area_.first.y*CELL_SIZE;
	short y1 = start_area_.second.y*CELL_SIZE;

	// interior

	glColor4f(1, 1, 1, .5);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	(ggl::vertex_array_flat<GLshort, 2>
		{ { x0, y0 }, { x1, y0 },
		  { x0, y1 }, { x1, y1 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_BLEND);

	// border

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
start_select_state::update(unsigned dpad_state)
{
	if (prev_dpad_state_ != ~0u &&
	  button_pressed(dpad_state, ggl::BUTTON1) && !button_pressed(prev_dpad_state_, ggl::BUTTON1)) {
		on_button_down();
		return;
	}

	prev_dpad_state_ = dpad_state;

	if (--change_tics_ == 0)
		reset_start_area();
}

void
start_select_state::on_button_down()
{
	game_.reset_player(start_area_.first);
	game_.fill_grid(start_area_.first, start_area_.second);

	game_.add_boss();

	game_.set_state(std::unique_ptr<game_state>(new playing_state { game_ }));
}

void
start_select_state::reset_start_area()
{
	static const int BORDER = 8;

	const int screen_cols = game_.viewport_width/CELL_SIZE;
	const int screen_rows = game_.viewport_height/CELL_SIZE;

	const vec2i v0 = -game_.offset/CELL_SIZE;

	const vec2i v1 {
		std::min(game_.grid_cols, v0.x + screen_cols),
		std::min(game_.grid_rows, v0.y + screen_rows) };

	vec2i from { rand(v0.x + BORDER, v1.x - BORDER), rand(v0.y + BORDER, v1.y - BORDER) };
	vec2i to { rand(from.x + 1, v1.x - BORDER + 1), rand(from.y + 1, v1.y - BORDER + 1) };

	start_area_ = std::make_pair(from, to);

	change_tics_ = 5;
}

//
//  p l a y i n g _ s t a t e
//

playing_state::playing_state(game& g)
: game_state { g }
, scrolling_ { false }
{ }

void
playing_state::draw() const
{
	game_.draw_border();
	game_.draw_foes();
	game_.draw_player();
}

void
playing_state::update(unsigned dpad_state)
{
	game_.update_player(dpad_state);
	game_.update_foes();

	// scrolling

	if (scrolling_) {
		static const int SCROLL_TICS = 30;

		if (++scroll_tics_ >= SCROLL_TICS) {
			game_.offset = next_offset_;
			scrolling_ = false;
		} else {
			game_.offset =
				quadratic_tween<vec2f>()(
					vec2f(prev_offset_),
					vec2f(next_offset_),
					static_cast<float>(scroll_tics_)/SCROLL_TICS);
		}
	} else {
		static const int SCROLL_DIST = 100;

		auto w = game_.viewport_width;
		auto h = game_.viewport_height;

		vec2i pos = game_.get_player_screen_position();

		if (pos.x < .2*w) {
			if (game_.offset.x < 0) {
				next_offset_.x = std::min(0, game_.offset.x + SCROLL_DIST);
				next_offset_.y = game_.offset.y;

				prev_offset_ = game_.offset;

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		} else if (pos.x > .8*w) {
			if (game_.offset.x + game_.grid_cols*CELL_SIZE > w) {
				next_offset_.x = std::max(w - game_.grid_cols*CELL_SIZE, game_.offset.x - SCROLL_DIST);
				next_offset_.y = game_.offset.y;

				prev_offset_ = game_.offset;

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		}

		if (pos.y < .2*h) {
			if (game_.offset.y < 0) {
				next_offset_.x = game_.offset.x;
				next_offset_.y = std::min(0, game_.offset.y + SCROLL_DIST);

				prev_offset_ = game_.offset;

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		} else if (pos.y > .8*h) {
			if (game_.offset.y + game_.grid_rows*CELL_SIZE > h) {
				next_offset_.x = game_.offset.x;
				next_offset_.y = std::max(h - game_.grid_rows*CELL_SIZE, game_.offset.y - SCROLL_DIST);

				prev_offset_ = game_.offset;

				scroll_tics_ = 0;
				scrolling_ = true;
			}
		}
	}
}

} // (anonymous namespace)

game_state::game_state(game& g)
: game_ { g }
{ }

game::game(int width, int height)
: viewport_width { width }
, viewport_height { height }
, player_ { *this }
{ }

void
game::reset(const level *l)
{
	cur_level = l;

	grid_rows = cur_level->grid_rows;
	grid_cols = cur_level->grid_cols;

	grid.resize(grid_rows*grid_cols);
	std::fill(std::begin(grid), std::end(grid), 0);

	offset = vec2i { 0, 0 };
	cover_percent_ = 0u;

	update_background();

	set_state(std::unique_ptr<game_state>(new level_intro_state(*this)));
}

void
game::draw() const
{
	glPushMatrix();
	glTranslatef(offset.x, offset.y, 0);

	draw_background();
	state_->draw();

	glPopMatrix();
}

void
game::update_background()
{
	auto& tex = cur_level->fg_texture;

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
game::update_border()
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

	cur_level->fg_texture->bind();
	background_filled_va_.draw(GL_TRIANGLES);

	cur_level->bg_texture->bind();
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
game::update(unsigned dpad_state)
{
	state_->update(dpad_state);
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

	// -1 --> 0
	//  0 --> 1
	std::transform(std::begin(grid), std::end(grid), std::begin(grid), [](int v) { return std::min(v + 1, 1); });

	update_border();
	update_background();
	update_cover_percent();
}

void
game::fill_grid(const vec2i& bottom_left, const vec2i& top_right)
{
	for (int r = bottom_left.y; r < top_right.y; r++) {
		auto *row = &grid[r*grid_cols];
		std::fill(row + bottom_left.x, row + top_right.x, 1);
	}

	update_border();
	update_background();
	update_cover_percent();
}

void
game::update_cover_percent()
{
	unsigned cover = 0;

	for (size_t i = 0; i < grid_rows*grid_cols; i++) {
		if (grid[i]) {
			cover += cur_level->silhouette[i];
		}
	}

	cover_percent_ = (static_cast<unsigned long long>(cover)*10000ull)/cur_level->silhouette_pixels;
}

unsigned
game::get_cover_percent() const
{
	return cover_percent_;
}

vec2i
game::get_player_screen_position() const
{
	return player_.get_position() + offset;
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
game::add_boss()
{
	// find initial boss position

	const int screen_cols = viewport_width/CELL_SIZE;
	const int screen_rows = viewport_height/CELL_SIZE;

	const int boss_cells = 2*boss::RADIUS/CELL_SIZE;

	const vec2i v0 = -offset/CELL_SIZE;
	const vec2i v1 {
		std::min(grid_cols, v0.x + screen_cols),
		std::min(grid_rows, v0.y + screen_rows) };

	vec2f boss_pos;

	int index = 1;

	for (int r = v0.y; r < v1.y - boss_cells; r++) {
		for (int c = v0.x; c < v1.x - boss_cells; c++) {
			bool filled = false;

			for (int i = 0; i < boss_cells; i++) {
				auto *begin = &grid[(r + i)*grid_cols + c];
				auto *end = begin + boss_cells;

				if (std::find(begin, end, 1) != end) {
					filled = true;
					break;
				}
			}

			if (!filled) {
				if (rand(0, index) == 0)
					boss_pos = vec2f { c, r }*CELL_SIZE + vec2f { boss::RADIUS, boss::RADIUS };

				++index;
			}
		}
	}

	add_foe(std::unique_ptr<foe> { new boss { *this, boss_pos } });
}

void
game::set_state(std::unique_ptr<game_state> next_state)
{
	state_ = std::move(next_state);
}

void
game::draw_foes() const
{
	for (auto& foe : foes)
		foe->draw();
}

void
game::draw_player() const
{
	player_.draw();
}

void
game::update_foes()
{
	auto it = std::begin(foes);

	while (it != std::end(foes)) {
		if (!(*it)->update())
			it = foes.erase(it);
		else
			++it;
	}

}

void
game::reset_player(const vec2i& pos)
{
	player_.reset(pos);
}

void
game::update_player(unsigned dpad_state)
{
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
}

#include <queue>
#include <algorithm>
#include <cassert>

#include <ggl/gl.h>
#include <ggl/dpad_button.h>
#include <ggl/texture.h>
#include <ggl/font.h>
#include <ggl/vertex_array.h>
#include <ggl/resources.h>
#include <ggl/program.h>
#include <ggl/action.h>
#include <ggl/render.h>
#include <ggl/util.h>
#include <ggl/window.h>
#include <ggl/tween.h>
#include <ggl/audio_player.h>

#include "util.h"
#include "level.h"
#include "boss.h"
#include "miniboss.h"
#include "percent_widget.h"
#include "lives_widget.h"
#include "dpad_widget.h"
#include "game.h"

namespace {

const int BORDER_RADIUS = 6;

class level_intro_state : public game_state
{
public:
	level_intro_state(game& g);

	void update(unsigned dpad_state) override;
	void draw() const override;
	void draw_overlay() const override;

private:
	const ggl::font *border_font_;
	const ggl::font *yellow_font_;
	const ggl::font *red_font_;

	float text_alpha_;
	float shadow_alpha_;
	float shadow_offset_;

	ggl::action_ptr action_;
};

class select_initial_offset_state : public game_state
{
public:
	select_initial_offset_state(game& g);

	void update(unsigned dpad_state) override;
	void draw() const override;
	void draw_overlay() const override;

private:
	static const int SCROLL_TICS = 30;

	int scroll_tics_, total_tics_;
	bool scroll_dir_;
	unsigned prev_dpad_state_;
};

class select_initial_area_state : public game_state
{
public:
	select_initial_area_state(game& g);

	void update(unsigned dpad_state) override;
	void draw() const override;
	void draw_overlay() const override;

private:
	void reset_initial_area();

	std::pair<vec2i, vec2i> initial_area_;

	int change_tics_, total_tics_;
	unsigned prev_dpad_state_;
};

class playing_state : public game_state
{
public:
	playing_state(game& g);

	void update(unsigned dpad_state) override;
	void draw() const override;
	void draw_overlay() const override;

private:
	void update_scroll();

	bool scrolling_;
	int scroll_tics_;
	vec2i prev_offset_, next_offset_; // when scrolling
};

class level_completed_state : public game_state
{
public:
	level_completed_state(game& g);

	void update(unsigned dpad_state) override;
	void draw() const override;
	void draw_overlay() const override;
};

class game_over_state : public game_state
{
public:
	game_over_state(game& g);

	void update(unsigned dpad_state) override;
	void draw() const override;
	void draw_overlay() const override;

private:
	const ggl::font *font_;
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
, border_font_ { ggl::res::get_font("fonts/title-border.spr") }
, yellow_font_ { ggl::res::get_font("fonts/title-yellow.spr") }
, red_font_ { ggl::res::get_font("fonts/title-red.spr") }
, action_ { ggl::res::get_action("animations/level-intro.xml") }
{
	action_->bind("text-alpha", &text_alpha_);
	action_->bind("shadow-offset", &shadow_offset_);
	action_->bind("shadow-alpha", &shadow_alpha_);
	action_->set_properties();
}

void
level_intro_state::draw() const
{ }

void
level_intro_state::draw_overlay() const
{
	const std::wstring stage_text { L"STAGE 1" };
	const std::wstring start_text { L"START !!" };

	auto w = game_.viewport_width;
	auto h = game_.viewport_height;

	const float y0 = .5f*h + 48;
	const float y1 = .5f*h - 48;

	// border

	ggl::render::set_color({ 1, 1, 1, shadow_alpha_ });

	border_font_->draw(0, stage_text, { .5f*w + shadow_offset_, y0 });
	border_font_->draw(0, stage_text, { .5f*w - shadow_offset_, y0 });

	border_font_->draw(0, start_text, { .5f*w + shadow_offset_, y1 });
	border_font_->draw(0, start_text, { .5f*w - shadow_offset_, y1 });

	// text

	ggl::render::set_color({ 1, 1, 1, text_alpha_ });
	yellow_font_->draw(1, stage_text, { .5f*w, y0 });
	red_font_->draw(1, start_text, { .5f*w, y1 });
}

void
level_intro_state::update(unsigned dpad_state)
{
	action_->update();

	if (action_->done())
		game_.enter_select_initial_offset_state();
}

//
//  o f f s e t _ s e l e c t _ s t a t e
//

select_initial_offset_state::select_initial_offset_state(game& g)
: game_state { g }
, scroll_tics_ { SCROLL_TICS }
, total_tics_ { 0 }
, scroll_dir_ { true }
, prev_dpad_state_ { ~0u }
{ }

void
select_initial_offset_state::draw() const
{ }

void
select_initial_offset_state::draw_overlay() const
{ }

void
select_initial_offset_state::update(unsigned dpad_state)
{
	if (++total_tics_ == 3*60 ||
	  (prev_dpad_state_ != ~0u &&
	   button_pressed(dpad_state, ggl::BUTTON1) && !button_pressed(prev_dpad_state_, ggl::BUTTON1))) {
		game_.enter_select_initial_area_state();
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

	game_.offset = from + (to - from)*ggl::tween::in_quadratic(static_cast<float>(scroll_tics_)/SCROLL_TICS);
}

//
//  s t a r t _ s e l e c t _ s t a t e
//

select_initial_area_state::select_initial_area_state(game& g)
: game_state { g }
, total_tics_ { 0 }
, prev_dpad_state_ { ~0u }
{
	reset_initial_area();
}

void
select_initial_area_state::draw() const
{
	ggl::render::set_color(ggl::rgba { 1, 1, 1, .5 });
	ggl::render::draw(ggl::bbox { initial_area_.first*CELL_SIZE, initial_area_.second*CELL_SIZE }, 0);
}

void
select_initial_area_state::draw_overlay() const
{ }

void
select_initial_area_state::update(unsigned dpad_state)
{
	if (++total_tics_ == 3*60 ||
	  (prev_dpad_state_ != ~0u &&
	   button_pressed(dpad_state, ggl::BUTTON1) && !button_pressed(prev_dpad_state_, ggl::BUTTON1))) {
		game_.enter_playing_state(initial_area_.first, initial_area_.second);
		return;
	}

	prev_dpad_state_ = dpad_state;

	if (--change_tics_ == 0)
		reset_initial_area();
}

void
select_initial_area_state::reset_initial_area()
{
	static const int BORDER = 1;
	static const int MAX_INITIAL_AREA = 200;

	const int screen_cols = game_.viewport_width/CELL_SIZE;
	const int screen_rows = game_.viewport_height/CELL_SIZE;

	const vec2i v0 = -game_.offset/CELL_SIZE;

	const vec2i v1 {
		std::min(game_.grid_cols, v0.x + screen_cols),
		std::min(game_.grid_rows, v0.y + screen_rows) };

	do {
		vec2i from { rand(v0.x + BORDER, v1.x - BORDER), rand(v0.y + BORDER, v1.y - BORDER) };
		vec2i to { rand(from.x + 1, v1.x - BORDER + 1), rand(from.y + 1, v1.y - BORDER + 1) };

		initial_area_ = std::make_pair(from, to);
	} while ((initial_area_.second.x - initial_area_.first.x)*
		 (initial_area_.second.y - initial_area_.first.y) > MAX_INITIAL_AREA);
	// XXX don't need this ugly loop, just pick a width then max height = MAX_AREA/width

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
	game_.draw_player();
}

void
playing_state::draw_overlay() const
{ }

void
playing_state::update(unsigned dpad_state)
{
	update_scroll();

	if (!game_.update_player(dpad_state))
		game_.enter_game_over_state();
}

void
playing_state::update_scroll()
{
	if (scrolling_) {
		static const int SCROLL_TICS = 30;

		if (++scroll_tics_ >= SCROLL_TICS) {
			game_.offset = next_offset_;
			scrolling_ = false;
		} else {
			game_.offset =
				prev_offset_ +
				(next_offset_ - prev_offset_)*
					ggl::tween::in_quadratic(static_cast<float>(scroll_tics_)/SCROLL_TICS);
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

//
//  l e v e l _ c o m p l e t e d _ s t a t e
//

level_completed_state::level_completed_state(game& g)
: game_state { g }
{ }

void
level_completed_state::draw() const
{ }

void
level_completed_state::draw_overlay() const
{ }

void
level_completed_state::update(unsigned dpad_state)
{ }

//
//  g a m e _ o v e r _ s t a t e
//

game_over_state::game_over_state(game& g)
: game_state { g }
, font_ { ggl::res::get_font("fonts/title-red.spr") }
{ }

void
game_over_state::draw() const
{ }

void
game_over_state::draw_overlay() const
{
	ggl::render::set_color(ggl::white);
	font_->draw(1, L"GAME OVER", { .5f*game_.viewport_width, .5f*game_.viewport_height });
}

void
game_over_state::update(unsigned dpad_state)
{ }

} // (anonymous namespace)

game_state::game_state(game& g)
: game_ { g }
{ }

game::game(int width, int height, bool virtual_dpad)
: viewport_width { width }
, viewport_height { height }
, dpad_state_ { 0 }
, player_ { *this }
, border_texture_ { ggl::res::get_texture("images/border.png") }
, flash_program_ { ggl::res::get_program("screenflash") }
, render_target_0_ { viewport_width, viewport_height }
, render_target_1_ { viewport_width, viewport_height }
, music_player_ { std::move(ggl::g_core->get_audio_player()) }
{
	widgets_.emplace_back(new percent_widget(*this));
	widgets_.emplace_back(new lives_widget(*this));

	using namespace std::placeholders;

	if (virtual_dpad) {
		std::unique_ptr<dpad_widget> dpad(new dpad_widget(*this));

		dpad_button_down_conn_ =
			dpad->get_dpad_button_down_event().connect(
					std::bind(&game::on_dpad_button_down, this, _1));

		dpad_button_up_conn_ =
			dpad->get_dpad_button_up_event().connect(
					std::bind(&game::on_dpad_button_up, this, _1));

		widgets_.push_back(std::move(dpad));
	} else {
		auto core = ggl::g_core;

		dpad_button_down_conn_ =
			core->get_dpad_button_down_event().connect(
					std::bind(&game::on_dpad_button_down, this, _1));

		dpad_button_up_conn_ =
			core->get_dpad_button_up_event().connect(
					std::bind(&game::on_dpad_button_up, this, _1));
	}
}

void
game::reset(const level *l)
{
	cur_level = l;

	grid_rows = cur_level->grid_rows;
	grid_cols = cur_level->grid_cols;

	grid.resize(grid_rows*grid_cols);
	std::fill(std::begin(grid), std::end(grid), 0);

	offset = vec2i { 0, -(grid_rows*CELL_SIZE - viewport_height) };
	cover_percent_ = 0u;

	update_background();

	// enter_level_intro_state();
	enter_select_initial_area_state();

	tics = 0;

	music_player_->open("music/music.ogg");
}

vec2f
game::get_viewport_offset() const
{
	vec2f o = offset;

	if (shake_tics_ > 0) {
		float t = static_cast<float>(shake_tics_)/shake_ttl_;
		float d = shake_intensity_*cosf(.75f*(shake_ttl_ - shake_tics_));
		o += shake_dir_*t*d;
	}

	return o;
}

void
game::draw() const
{
	render_target_0_.bind();
	draw_scene();

	if (post_filters_.empty()) {
		passthru_filter_.draw(render_target_0_, ggl::window());
	} else {
		const ggl::framebuffer *source = &render_target_0_, *dest = &render_target_1_;
		ggl::window window;

		const unsigned num_filters = post_filters_.size();

		for (unsigned i = 0; i < num_filters; i++) {
			post_filters_[i]->draw(*source, i < num_filters - 1 ? *static_cast<const ggl::render_target *>(dest) : window);
			std::swap(source, dest);
		}
	}

	if (flash_tics_ > 0) {
		ggl::enable_additive_blend _;

		float t = static_cast<float>(flash_tics_)/flash_ttl_;

		static const ggl::vertex_array_flat<GLshort, 2> va { { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };

		flash_program_->use();
		flash_program_->set_uniform_f("t", t);
		va.draw(GL_TRIANGLE_STRIP);
	}
}

void
game::draw_scene() const
{
	// relative to grid

	vec2f o = get_viewport_offset();

	ggl::render::set_viewport(
		{ { -o.x, -o.y },
		  { viewport_width - o.x, viewport_height - o.y } });

	draw_background();

	ggl::render::begin();

	state_->draw();

	for (auto& e : entities)
		e->draw();

	for (auto& e : effects_) {
		if (!e->is_position_absolute())
			e->draw();
	}

	ggl::render::end();

	// relative to screen

	ggl::render::set_viewport({ { 0, 0 }, { viewport_width, viewport_height } });

	ggl::render::begin();

	state_->draw_overlay();

	for (auto& w : widgets_)
		w->draw();

	for (auto& e : effects_) {
		if (e->is_position_absolute())
			e->draw();
	}

	ggl::render::end();
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

		border_va_.push_back({ p0.x, p0.y, 0, static_cast<short>(i) });
		border_va_.push_back({ p1.x, p1.y, 1, static_cast<short>(i) });
	}
}

void
game::draw_background() const
{
	auto prog = ggl::res::get_program("texture");
	prog->use();
	prog->set_uniform_mat4("proj_modelview", ggl::render::get_proj_modelview());

	cur_level->fg_texture->bind();
	background_filled_va_.draw(GL_TRIANGLES);

	cur_level->bg_texture->bind();
	background_unfilled_va_.draw(GL_TRIANGLES);

	border_texture_->bind();

	ggl::enable_alpha_blend _;
	border_va_.draw(GL_TRIANGLE_STRIP);
}

void
game::update()
{
	++tics;

	// entities
	for (auto it = std::begin(entities); it != std::end(entities); ) {
		if (!(*it)->update())
			it = entities.erase(it);
		else
			++it;
	}

	// hud
	for (auto& w : widgets_)
		w->update();

	// effects
	for (auto it = std::begin(effects_); it != std::end(effects_); ) {
		if (!(*it)->update())
			it = effects_.erase(it);
		else
			++it;
	}

	// post-filters
	for (auto it = std::begin(post_filters_); it != std::end(post_filters_); ) {
		if (!(*it)->update())
			it = post_filters_.erase(it);
		else
			++it;
	}

	state_->update(dpad_state_);

	if (shake_tics_ > 0)
		--shake_tics_;

	if (flash_tics_ > 0)
		--flash_tics_;

	music_player_->update();
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

	vec2i pos = (vec2i(cur_boss_->get_position()) + vec2i { CELL_SIZE, CELL_SIZE }/2)/CELL_SIZE;

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
						{ return (pos == t.first && next_pos == t.second) ||
							 (pos == t.second && next_pos == t.first); });

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

	cover_update_event_.notify(cover_percent_);
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
game::add_entity(std::unique_ptr<entity> e)
{
	entities.push_back(std::move(e));
}

void
game::add_effect(std::unique_ptr<effect> e)
{
	effects_.push_back(std::move(e));
}

void
game::add_post_filter(std::unique_ptr<dynamic_post_filter> f)
{
	post_filters_.push_back(std::move(f));
}

void
game::start_screenshake(int duration, float intensity)
{
	shake_tics_ = shake_ttl_ = duration;
	shake_intensity_ = intensity;

	float a = rand<float>(0.f, 2.f*M_PI);
	shake_dir_ = { sinf(a), cosf(a) };
}

void
game::start_screenflash(int duration)
{
	flash_tics_ = flash_ttl_ = duration;
}

vec2f
game::find_foe_pos(int radius) const
{
	const int screen_cols = viewport_width/CELL_SIZE;
	const int screen_rows = viewport_height/CELL_SIZE;

	const int cells = 2*radius/CELL_SIZE;

	const vec2i v0 = -offset/CELL_SIZE;
	const vec2i v1 {
		std::min(grid_cols, v0.x + screen_cols),
		std::min(grid_rows, v0.y + screen_rows) };

	vec2f pos;

	int index = 1;

	for (int r = v0.y; r < v1.y - cells; r++) {
		for (int c = v0.x; c < v1.x - cells; c++) {
			bool filled = false;

			for (int i = 0; i < cells; i++) {
				auto *begin = &grid[(r + i)*grid_cols + c];
				auto *end = begin + cells;

				if (std::find(begin, end, 1) != end) {
					filled = true;
					break;
				}
			}

			if (!filled) {
				if (rand(0, index) == 0)
					pos = vec2f { c, r }*CELL_SIZE + vec2f { radius, radius };

				++index;
			}
		}
	}

	return pos;
}

void
game::add_foes()
{
	std::unique_ptr<entity> e { new boss { *this, find_foe_pos(boss::RADIUS) } };
	cur_boss_ = static_cast<foe *>(e.get());
	add_entity(std::move(e));

	for (int i = 0; i < 5; i++)
		add_entity(std::unique_ptr<entity> { new miniboss { *this, find_foe_pos(miniboss::RADIUS) } });

}

void
game::enter_level_intro_state()
{
	state_ = std::unique_ptr<game_state>(new level_intro_state { *this });
}

void
game::enter_select_initial_offset_state()
{
	state_ = std::unique_ptr<game_state>(new select_initial_offset_state { *this });
}

void
game::enter_select_initial_area_state()
{
	state_ = std::unique_ptr<game_state>(new select_initial_area_state { *this });
}

void
game::enter_playing_state(const vec2i& bottom_left, const vec2i& top_right)
{
	reset_player(bottom_left);
	fill_grid(bottom_left, top_right);

	add_foes();

	state_ = std::unique_ptr<game_state>(new playing_state { *this });

	start_event_.notify();

	music_player_->start();
}

void
game::enter_game_over_state()
{
	state_ = std::unique_ptr<game_state>(new game_over_state { *this });

	stop_event_.notify();

	music_player_->fade_out(2*60);
}

void
game::enter_level_completed_state()
{
	state_ = std::unique_ptr<game_state>(new level_completed_state { *this });
}

void
game::draw_player() const
{
	player_.draw();
}

player&
game::get_player()
{
	return player_;
}

void
game::reset_player(const vec2i& pos)
{
	player_.reset(pos);
}

bool
game::update_player(unsigned dpad_state)
{
	return player_.update(dpad_state);
}

ggl::connectable_event<game::cover_update_event_handler>&
game::get_cover_update_event()
{
	return cover_update_event_;
}

ggl::connectable_event<game::start_event_handler>&
game::get_start_event()
{
	return start_event_;
}

ggl::connectable_event<game::stop_event_handler>&
game::get_stop_event()
{
	return stop_event_;
}

void
game::on_dpad_button_down(ggl::dpad_button button)
{
	dpad_state_ |= (1u << static_cast<int>(button));
}

void
game::on_dpad_button_up(ggl::dpad_button button)
{
	dpad_state_ &= ~(1u << static_cast<int>(button));
}

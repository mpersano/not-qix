#include <sstream>

#include <ggl/resources.h>
#include <ggl/font.h>
#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/action.h>
#include <ggl/render.h>
#include <ggl/util.h>
#include <ggl/tween.h>

#include "game.h"
#include "effect.h"
#include "util.h"
#include "bezier.h"
#include "particles.h"
#include "percent_widget.h"

namespace {

static const int INTRO_TICS = 10;
static const int OUTRO_TICS = 10;
static const int UPDATE_TICS = 20;

class update_effect : public effect
{
public:
	update_effect(const vec2f& start_pos, const vec2f& end_pos, unsigned value);

	void draw() const override;

	bool is_position_absolute() const override
	{ return true; }

private:
	bool do_update() override;

	std::unique_ptr<bezier<vec2f>> path_;

	std::wstring text_;
	const ggl::font *font_;

	float path_u_;
	float text_alpha_;
	float text_scale_;

	ggl::action_ptr action_;
};

update_effect::update_effect(const vec2f& start_pos, const vec2f& end_pos, unsigned value)
: font_ { ggl::res::get_font("fonts/powerup.spr") }
, path_u_ { 0 }
, text_alpha_ { 1 }
, text_scale_ { 1 }
, action_ { ggl::res::get_action("animations/percent-update.xml") }
{
	// text

	std::basic_stringstream<wchar_t> ss;
	ss << "+" << (value/100) << "." << (value%100) << "%";
	text_ = ss.str();

	// path

	auto d = end_pos - start_pos;

	auto cm = .5f*(end_pos + start_pos);
	auto cn = normalized(vec2f { -d.y, d.x });

	float l = length(d);

	cm += cn*rand(-.5*l, .5*l);

	path_.reset(new bezier<vec2f> { start_pos, cm, end_pos });

	// action

	action_->bind("u", &path_u_);
	action_->bind("alpha", &text_alpha_);
	action_->bind("scale", &text_scale_);
	action_->set_properties();
}

bool
update_effect::do_update()
{
	action_->update();
	return !action_->done();
}

void
update_effect::draw() const
{
	auto pos = (*path_)(path_u_);

	ggl::render::set_color({ 1, 1, 1, text_alpha_ });

	ggl::render::push_matrix();
	ggl::render::translate(pos.x, pos.y);
	ggl::render::scale(text_scale_);
	font_->draw(1, text_);
	ggl::render::pop_matrix();
}

}

percent_widget::percent_widget(game& g)
: widget { g }
, position_top_ { true }
, cur_value_ { 0 }
, next_value_ { 0 }
, large_font_ { ggl::res::get_font("fonts/hud-big.spr") }
, small_font_ { ggl::res::get_font("fonts/hud-small.spr") }
, frame_ { ggl::res::get_sprite("percent-frame.png") }
, updating_ { false }
, update_tics_ { 0 }
, hidden_ { true }
{
	cover_update_conn_ =
		game_.get_cover_update_event().connect(
			std::bind(&percent_widget::on_cover_update, this, std::placeholders::_1));

	game_start_conn_ =
		game_.get_start_event().connect(
			std::bind(&percent_widget::on_game_start, this));

	game_stop_conn_ =
		game_.get_stop_event().connect(
			std::bind(&percent_widget::on_game_stop, this));

	set_state(state::HIDDEN);
}

void
percent_widget::on_game_stop()
{
	hidden_ = true;
}

void
percent_widget::on_game_start()
{
	hidden_ = false;
}

void
percent_widget::set_state(state next_state)
{
	state_ = next_state;
	state_tics_ = 0;
}

bool
percent_widget::update()
{
	switch (state_) {
		case state::INTRO:
			if (++state_tics_ >= INTRO_TICS)
				set_state(state::IDLE);
			break;

		case state::IDLE:
			if (hidden_) {
				set_state(state::OUTRO);
			} else {
				int player_y = game_.get_player_screen_position().y;

				if (position_top_) {
					if (player_y > .7*game_.viewport_height) {
						set_state(state::OUTRO);
					}
				} else {
					if (player_y < .3*game_.viewport_height) {
						set_state(state::OUTRO);
					}
				}
			}
			break;

		case state::OUTRO:
			if (++state_tics_ >= OUTRO_TICS) {
				if (!hidden_) {
					position_top_ = !position_top_;
					set_state(state::INTRO);
				} else {
					set_state(state::HIDDEN);
				}
			}
			break;

		case state::HIDDEN:
			if (!hidden_)
				set_state(state::INTRO);
			break;
	}

	if (updating_) {
		if (++update_tics_ >= UPDATE_TICS) {
			cur_value_ = next_value_;
			updating_ = false;
		}
	}

	return true;
}

void
percent_widget::on_cover_update(unsigned percent)
{
	if (percent > cur_value_) {
		auto pos = game_.get_player_screen_position();

		auto e = std::unique_ptr<effect> {
			new update_effect {
				pos,
				vec2f { get_base_x() + .5f*frame_->width, get_base_y() + .5f*frame_->height },
				percent - cur_value_ } };

		effect_finished_conn_ =
			static_cast<update_effect *>(e.get())->get_finished_event().connect([=]()
				{
					next_value_ = percent;
					updating_ = true;
					update_tics_ = 0;
				});

		game_.add_effect(std::move(e));

		const gradient particle_colors { { 1.f, .5f, 0.f }, { 1.f, 1.f, 0.f }, { 1.f, 1.f, 1.f } };
		game_.add_effect(std::unique_ptr<effect> { new particles { pos, 20, particle_colors } });
	}
}

int
percent_widget::get_base_y() const
{
	if (position_top_)
		return game_.viewport_height - frame_->height;
	else
		return 0;
}

int
percent_widget::get_base_x() const
{
	const int w = frame_->width;

	switch (state_) {
		case state::HIDDEN:
			return -w;

		case state::INTRO:
			return -w*(1.f - ggl::tween::in_quadratic(static_cast<float>(state_tics_)/INTRO_TICS));

		case state::IDLE:
			return 0;

		case state::OUTRO:
			return -w*ggl::tween::in_quadratic(static_cast<float>(state_tics_)/OUTRO_TICS);
	}
}

unsigned
percent_widget::get_value() const
{
	if (updating_)
		return cur_value_ + (next_value_ - cur_value_)*ggl::tween::in_quadratic(static_cast<float>(update_tics_)/UPDATE_TICS);
	else
		return cur_value_;
}

void
percent_widget::draw() const
{
	ggl::render::set_color(ggl::white);

	draw_frame();
	draw_digits();
}

void
percent_widget::draw_frame() const
{
	frame_->draw(
		0,
		vec2f { get_base_x(), get_base_y() },
		ggl::vert_align::BOTTOM, ggl::horiz_align::LEFT);
}

void
percent_widget::draw_digits() const
{
	const int BIG_DIGIT_WIDTH = 32;
	const int SMALL_DIGIT_WIDTH = 28;

	const unsigned value = get_value();

	const unsigned int_part = value/100;
	const unsigned fract_part = value%100;

	const int base_y = get_base_y();
	const int base_x = get_base_x();

	int x = get_base_x() + 12 + BIG_DIGIT_WIDTH/2;
	draw_char(large_font_, L'0' + int_part%10, x + 2*BIG_DIGIT_WIDTH, base_y);
	if (int_part >= 10) {
		draw_char(large_font_, L'0' + (int_part/10)%10, x + BIG_DIGIT_WIDTH, base_y);
		if (int_part >= 100) {
			draw_char(large_font_, L'0' + (int_part/100)%10, x, base_y);
		}
	}

	x += 90;
	draw_char(small_font_, L'.', x, base_y);

	x += 23;
	draw_char(small_font_, L'0' + (fract_part/10)%10, x, base_y);
	draw_char(small_font_, L'0' + (fract_part)%10, x + SMALL_DIGIT_WIDTH, base_y);
	draw_char(small_font_, L'%', x + 2*SMALL_DIGIT_WIDTH + 16, base_y);
}

void
percent_widget::draw_char(const ggl::font *f, wchar_t ch, int base_x, int base_y) const
{
	auto *g = f->get_glyph(ch);
	vec2f pos = vec2f { base_x, base_y + 32 + g->top - g->spr.height };
	g->spr.draw(1, pos, ggl::vert_align::BOTTOM, ggl::horiz_align::CENTER);
}

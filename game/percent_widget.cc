#include <sstream>

#include <ggl/resources.h>
#include <ggl/font.h>
#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/action.h>
#include <ggl/util.h>

#include "tween.h"
#include "game.h"
#include "effect.h"
#include "quad.h"
#include "util.h"
#include "percent_widget.h"

namespace {

static const int INTRO_TICS = 10;
static const int OUTRO_TICS = 10;
static const int UPDATE_TICS = 20;

template <typename T>
struct bezier
{
	bezier(const T& c0, const T& c1, const T& c2)
	: c0 { c0 }, c1 { c1 }, c2 { c2 }
	{ }

	T operator()(float u) const
	{
		const float w0 = (1 - u)*(1 - u);
		const float w1 = 2*u*(1 - u);
		const float w2 = u*u;

		return c0*w0 + c1*w1 + c2*w2;
	}

	T c0, c1, c2;
};

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
	std::unique_ptr<text_quad> text_;

	float path_u_;
	float text_alpha_;
	float text_scale_;

	ggl::action_ptr action_;
};

update_effect::update_effect(const vec2f& start_pos, const vec2f& end_pos, unsigned value)
: path_u_ { 0 }
, text_alpha_ { 1 }
, text_scale_ { 1 }
, action_ { ggl::res::get_action("animations/percent-update.xml") }
{
	// text

	std::basic_stringstream<wchar_t> ss;
	ss << "+" << (value/100) << "." << (value%100) << "%";
	text_.reset(new text_quad { ggl::res::get_font("fonts/powerup.spr"), ss.str() });

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

	glColor4f(1, 1, 1, text_alpha_);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0.f);
	glScalef(text_scale_, text_scale_, 1.f);
	text_->draw();
	glPopMatrix();
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
	auto effect = std::unique_ptr<update_effect> {
			new update_effect {
				game_.get_player_screen_position(),
				vec2f { get_base_x() + .5f*frame_->width, get_base_y() + .5f*frame_->height },
				percent - cur_value_ } };

	effect_finished_conn_ =
		static_cast<update_effect *>(effect.get())->get_finished_event().connect(
			[=]() {
				next_value_ = percent;
				updating_ = true;
				update_tics_ = 0; });

	game_.add_effect(std::move(effect));
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
			return -w*(1.f - quadratic_tween(static_cast<float>(state_tics_)/INTRO_TICS));

		case state::IDLE:
			return 0;

		case state::OUTRO:
			return -w*quadratic_tween(static_cast<float>(state_tics_)/OUTRO_TICS);
	}
}

unsigned
percent_widget::get_value() const
{
	if (updating_)
		return cur_value_ + (next_value_ - cur_value_)*quadratic_tween(static_cast<float>(update_tics_)/UPDATE_TICS);
	else
		return cur_value_;
}

void
percent_widget::draw() const
{
	ggl::enable_alpha_blend _;

	draw_frame();
	draw_digits();
}

void
percent_widget::draw_frame() const
{
	frame_->draw(
		get_base_x(), get_base_y(),
		ggl::sprite::horiz_align::LEFT,
		ggl::sprite::vert_align::BOTTOM);
}

void
percent_widget::draw_digits() const
{
	ggl::enable_texture _;
	ggl::enable_alpha_blend __;

	const int BIG_DIGIT_WIDTH = 20;
	const int SMALL_DIGIT_WIDTH = 16;

	const unsigned value = get_value();

	const unsigned int_part = value/100;
	const unsigned fract_part = value%100;

	const int base_y = get_base_y();
	const int base_x = get_base_x();

	glColor4f(1, 1, 1, 1);

	int x = get_base_x() + 8 + BIG_DIGIT_WIDTH/2;
	draw_char(large_font_, L'0' + int_part%10, x + 2*BIG_DIGIT_WIDTH, base_y);
	if (int_part >= 10) {
		draw_char(large_font_, L'0' + (int_part/10)%10, x + BIG_DIGIT_WIDTH, base_y);
		if (int_part >= 100) {
			draw_char(large_font_, L'0' + (int_part/100)%10, x, base_y);
		}
	}

	x += 2*BIG_DIGIT_WIDTH + BIG_DIGIT_WIDTH/2 + SMALL_DIGIT_WIDTH/2;
	draw_char(small_font_, L'.', x, base_y);
	draw_char(small_font_, L'0' + (fract_part/10)%10, x + SMALL_DIGIT_WIDTH, base_y);
	draw_char(small_font_, L'0' + (fract_part)%10, x + 2*SMALL_DIGIT_WIDTH, base_y);
	draw_char(small_font_, L'%', x + 3*SMALL_DIGIT_WIDTH + 12, base_y);
}

void
percent_widget::draw_char(const ggl::font *f, wchar_t ch, int base_x, int base_y) const
{
	auto *g = f->get_glyph(ch);

	float x0 = base_x - .5f*g->spr.width;
	float x1 = x0 + g->spr.width;
	float y0 = base_y + 20 + g->top;
	float y1 = y0 - g->spr.height;

	const float u0 = g->spr.u0;
	const float u1 = g->spr.u1;
	const float v0 = g->spr.v0;
	const float v1 = g->spr.v1;

	g->spr.tex->bind();

	(ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { x0, y0, u0, v0 },
		  { x1, y0, u1, v0 },
		  { x0, y1, u0, v1 },
		  { x1, y1, u1, v1 } }).draw(GL_TRIANGLE_STRIP);
}

#include <ggl/resources.h>
#include <ggl/font.h>
#include <ggl/texture.h>

#include "tween.h"
#include "game.h"
#include "percent_gauge.h"

namespace {

static const float INTRO_T = 1;
static const float OUTRO_T = 1;
static const float UPDATE_T = .5;

}

percent_gauge::percent_gauge(game& g, int viewport_height)
: game_ { g }
, viewport_height_ { viewport_height }
, position_top_ { true }
, cur_value_ { 0 }
, large_font_ { ggl::res::get_font("fonts/small") }
, small_font_ { ggl::res::get_font("fonts/tiny") }
, updating_ { false }
, update_t_ { 0 }
{
	set_state(state::INTRO);
}

void
percent_gauge::set_state(state next_state)
{
	state_ = next_state;
	state_t_ = 0;
}

bool
percent_gauge::update(float dt)
{
	switch (state_) {
		case state::INTRO:
			if ((state_t_ += dt) >= INTRO_T) {
				state_ = state::IDLE;
			}
			break;

		case state::IDLE:
			{
			int player_y = game_.get_player_screen_position().y;

			if (position_top_) {
				if (player_y > .7*viewport_height_) {
					set_state(state::OUTRO);
				}
			} else {
				if (player_y < .3*viewport_height_) {
					set_state(state::OUTRO);
				}
			}
			}
			break;

		case state::OUTRO:
			if ((state_t_ += dt) >= OUTRO_T) {
				position_top_ = !position_top_;
				set_state(state::INTRO);
			}
			break;
	}

	if (updating_) {
		if ((update_t_ += dt) >= UPDATE_T) {
			cur_value_ = next_value_;
			updating_ = false;
		}
	} else {
		unsigned p = game_.get_cover_percent();

		if (p != cur_value_) {
			next_value_ = p;
			updating_ = true;
			update_t_ = 0;
		}
	}

	return true;
}

int
percent_gauge::get_base_y() const
{
	if (position_top_)
		return viewport_height_ - TOP_MARGIN - HEIGHT;
	else
		return TOP_MARGIN;
}

int
percent_gauge::get_base_x() const
{
	switch (state_) {
		case state::INTRO:
			return quadratic_tween<int>()(-WIDTH, LEFT_MARGIN, static_cast<float>(state_t_)/INTRO_T);

		case state::IDLE:
			return LEFT_MARGIN;

		case state::OUTRO:
			return quadratic_tween<int>()(LEFT_MARGIN, -WIDTH, static_cast<float>(state_t_)/OUTRO_T);
	}
}

unsigned
percent_gauge::get_value() const
{
	if (updating_)
		return exp_tween<unsigned>()(cur_value_, next_value_, static_cast<float>(update_t_)/UPDATE_T);
	else
		return cur_value_;
}

void
percent_gauge::draw() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	draw_frame();
	draw_digits();

	glDisable(GL_BLEND);
}

void
percent_gauge::draw_frame() const
{
	glColor4f(1, 0, 0, .6);

	const short x0 = get_base_x();
	const short x1 = x0 + WIDTH;
	const short y0 = get_base_y();
	const short y1 = y0 + HEIGHT;

	(ggl::vertex_array_flat<GLshort, 2>
		{ { x0, y0 }, { x1, y0 }, { x0, y1 }, { x1, y1 } }).draw(GL_TRIANGLE_STRIP);
}

void
percent_gauge::draw_digits() const
{

	const int BIG_DIGIT_WIDTH = 20;
	const int SMALL_DIGIT_WIDTH = 16;

	const unsigned value = get_value();

	const unsigned int_part = value/100;
	const unsigned fract_part = value%100;

	const int base_y = get_base_y();
	const int base_x = get_base_x();

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);

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
	draw_char(small_font_, L'%', x + 3*SMALL_DIGIT_WIDTH, base_y);

	glDisable(GL_TEXTURE_2D);
}

void
percent_gauge::draw_char(const ggl::font *f, wchar_t ch, int base_x, int base_y) const
{
	auto *g = f->find_glyph(ch);

	float x0 = base_x - .5f*g->width;
	float x1 = x0 + g->width;
	float y0 = base_y + 20 + g->top;
	float y1 = y0 - g->height;

	auto& t0 = g->texuv[0];
	auto& t1 = g->texuv[1];
	auto& t2 = g->texuv[2];
	auto& t3 = g->texuv[3];

	f->get_texture()->bind();

	(ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { x0, y0, t0.x, t0.y },
		  { x1, y0, t1.x, t1.y },
		  { x0, y1, t3.x, t3.y },
		  { x1, y1, t2.x, t2.y } }).draw(GL_TRIANGLE_STRIP);
}
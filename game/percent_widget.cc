#include <ggl/resources.h>
#include <ggl/font.h>
#include <ggl/texture.h>

#include "tween.h"
#include "game.h"
#include "percent_widget.h"

namespace {

static const int INTRO_TICS = 10;
static const int OUTRO_TICS = 10;
static const int UPDATE_TICS = 5;

}

percent_widget::percent_widget(game& g)
: widget { g }
, position_top_ { true }
, cur_value_ { 0 }
, large_font_ { ggl::res::get_font("fonts/small.spr") }
, small_font_ { ggl::res::get_font("fonts/tiny.spr") }
, updating_ { false }
, update_tics_ { 0 }
, hidden_ { true }
{
	set_state(state::HIDDEN);
}

void
percent_widget::hide()
{
	hidden_ = true;
}

void
percent_widget::show()
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
	} else {
		unsigned p = game_.get_cover_percent();

		if (p != cur_value_) {
			next_value_ = p;
			updating_ = true;
			update_tics_ = 0;
		}
	}

	return true;
}

int
percent_widget::get_base_y() const
{
	if (position_top_)
		return game_.viewport_height - TOP_MARGIN - HEIGHT;
	else
		return TOP_MARGIN;
}

int
percent_widget::get_base_x() const
{
	switch (state_) {
		case state::HIDDEN:
			return -WIDTH;

		case state::INTRO:
			return -WIDTH + (LEFT_MARGIN + WIDTH)*quadratic_tween(static_cast<float>(state_tics_)/INTRO_TICS);

		case state::IDLE:
			return LEFT_MARGIN;

		case state::OUTRO:
			return LEFT_MARGIN + (-WIDTH - LEFT_MARGIN)*quadratic_tween(static_cast<float>(state_tics_)/OUTRO_TICS);
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	draw_frame();
	draw_digits();

	glDisable(GL_BLEND);
}

void
percent_widget::draw_frame() const
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
percent_widget::draw_digits() const
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
percent_widget::draw_char(const ggl::font *f, wchar_t ch, int base_x, int base_y) const
{
	auto *g = f->find_glyph(ch);

	float x0 = base_x - .5f*g->width;
	float x1 = x0 + g->width;
	float y0 = base_y + 20 + g->top;
	float y1 = y0 - g->height;

	const float u0 = g->u0;
	const float u1 = g->u1;
	const float v0 = g->v0;
	const float v1 = g->v1;

	g->tex->bind();

	(ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { x0, y0, u0, v0 },
		  { x1, y0, u1, v0 },
		  { x0, y1, u0, v1 },
		  { x1, y1, u1, v1 } }).draw(GL_TRIANGLE_STRIP);
}
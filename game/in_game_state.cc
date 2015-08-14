#include <ggl/core.h>
#include <ggl/resources.h>
#include <ggl/font.h>
#include <ggl/texture.h>

#include "level.h"
#include "effect.h"
#include "in_game_state.h"

namespace {

const int MARGIN = 8;

class percentage_gauge : public effect
{
public:
	percentage_gauge(game& g, int viewport_height);

	bool update(float dt) override;
	void draw() const override;

private:
	void draw_frame() const;
	void draw_digits() const;

	static const int TOP_MARGIN = 20;
	static const int LEFT_MARGIN = 20;
	static const int WIDTH = 160;
	static const int HEIGHT = 60;

	game& game_;
	int viewport_height_;
	int base_y_;
	const ggl::font *large_font_, *small_font_;
};

percentage_gauge::percentage_gauge(game& g, int viewport_height)
: game_ { g }
, viewport_height_ { viewport_height }
, large_font_ { ggl::res::get_font("fonts/small") }
, small_font_ { ggl::res::get_font("fonts/tiny") }
, base_y_ { viewport_height - TOP_MARGIN - HEIGHT }
{ }

bool
percentage_gauge::update(float dt)
{
	return true;
}

void
percentage_gauge::draw() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	draw_frame();
	draw_digits();

	glDisable(GL_BLEND);
}

void
percentage_gauge::draw_frame() const
{
	glColor4f(1, 0, 0, .6);

	const short x0 = LEFT_MARGIN;
	const short x1 = LEFT_MARGIN + WIDTH;
	const short y0 = base_y_;
	const short y1 = base_y_ + HEIGHT;

	(ggl::vertex_array_flat<GLshort, 2>
		{ { x0, y0 }, { x1, y0 }, { x0, y1 }, { x1, y1 } }).draw(GL_TRIANGLE_STRIP);
}

void
percentage_gauge::draw_digits() const
{
	int y = base_y_ + 20;
	float x = LEFT_MARGIN + 8;

	auto draw_char = [&](const ggl::font* font, wchar_t ch, int width)
		{
			auto *g = font->find_glyph(ch);

			float x0 = x + .5f*(width - g->width);
			float x1 = x0 + g->width;
			float y0 = y + g->top;
			float y1 = y0 - g->height;

			auto& t0 = g->texuv[0];
			auto& t1 = g->texuv[1];
			auto& t2 = g->texuv[2];
			auto& t3 = g->texuv[3];

			font->get_texture()->bind();

			(ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
				{ { x0, y0, t0.x, t0.y },
				  { x1, y0, t1.x, t1.y },
				  { x0, y1, t3.x, t3.y },
				  { x1, y1, t2.x, t2.y } }).draw(GL_TRIANGLE_STRIP);

			x += width;
		};

	const int BIG_DIGIT_WIDTH = 20;
	const int SMALL_DIGIT_WIDTH = 16;

	const unsigned value = game_.get_cover_percentage();

	const unsigned int_part = value/100;
	const unsigned fract_part = value%100;

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);

	draw_char(large_font_, L'0' + (int_part/100)%10, BIG_DIGIT_WIDTH);
	draw_char(large_font_, L'0' + (int_part/10)%10, BIG_DIGIT_WIDTH);
	draw_char(large_font_, L'0' + int_part%10, BIG_DIGIT_WIDTH);

	draw_char(small_font_, L'.', SMALL_DIGIT_WIDTH/2);
	draw_char(small_font_, L'0' + (fract_part/10)%10, SMALL_DIGIT_WIDTH);
	draw_char(small_font_, L'0' + (fract_part)%10, SMALL_DIGIT_WIDTH);
	draw_char(small_font_, L'%', SMALL_DIGIT_WIDTH);

	glDisable(GL_TEXTURE_2D);
}

}

in_game_state::in_game_state(int width, int height)
: app_state { width, height }
, game_ { width - 2*MARGIN, height - 2*MARGIN }
{
	game_.reset(g_levels[0].get());
	effects_.push_back(std::unique_ptr<effect>(new percentage_gauge { game_, height }));
}

void
in_game_state::draw() const
{
	glPushMatrix();
	glTranslatef(MARGIN, MARGIN, 0);
	game_.draw();
	glPopMatrix();

	for (auto& p : effects_)
		p->draw();
}

void
in_game_state::update(float dt)
{
	update_game(dt);
	update_effects(dt);
}

void
in_game_state::update_game(float dt)
{
	unsigned dpad_state = ggl::g_core->get_dpad_state();

	if (dpad_state & ggl::DPAD_UP)
		game_.move(direction::UP, dpad_state & ggl::DPAD_BUTTON1);

	if (dpad_state & ggl::DPAD_DOWN)
		game_.move(direction::DOWN, dpad_state & ggl::DPAD_BUTTON1);

	if (dpad_state & ggl::DPAD_LEFT)
		game_.move(direction::LEFT, dpad_state & ggl::DPAD_BUTTON1);

	if (dpad_state & ggl::DPAD_RIGHT)
		game_.move(direction::RIGHT, dpad_state & ggl::DPAD_BUTTON1);

	game_.update(dt);
}

void
in_game_state::update_effects(float dt)
{
	auto it = effects_.begin();

	while (it != effects_.end()) {
		if (!(*it)->update(dt))
			it = effects_.erase(it);
		else
			++it;
	}
}

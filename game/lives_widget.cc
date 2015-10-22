#include <cassert>	
#include <sstream>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/texture.h>
#include <ggl/resources.h>

#include "game.h"
#include "lives_widget.h"

lives_widget::lives_widget(game& g)
: widget { g }
, show_tics_ { 0 }
, circle_sprite_ { ggl::res::get_sprite("lives-left-circle.png") }
, shine_texture_ { ggl::res::get_texture("images/lives-left-shine.png") }
{
	auto& p = game_.get_player();

	respawn_conn_ =
		p.get_respawn_event().connect(
			std::bind(&lives_widget::on_player_respawn, this, std::placeholders::_1));

	death_conn_ =
		p.get_death_event().connect(
			std::bind(&lives_widget::on_player_death, this));
}

void
lives_widget::initialize_text(int lives_left)
{
	std::basic_stringstream<wchar_t> ss;

	if (lives_left > 0)
		ss << lives_left << " left";
	else
		ss << "last player";

	text_.reset(new text_quad { ggl::res::get_font("fonts/small.spr"), ss.str() });
}

bool
lives_widget::update()
{
	if (show_tics_ > 0)
		--show_tics_;

	return true;
}

void
lives_widget::draw() const
{
	if (!show_tics_)
		return;

	auto pos = game_.get_player_screen_position();

	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0);

	// circle

	glPushMatrix();

	const float sx = pos.x > .5f*game_.viewport_width ? -1.f : 1.f;
	const float sy = pos.y > .5f*game_.viewport_height ? 1.f : -1.f;
	glScalef(sx, sy, 1);

	glColor4f(1, 1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const short w = circle_sprite_->width;
	const short h = circle_sprite_->height;

	const float x0 = -.5f*w;
	const float x1 = x0 + w;

	const float y0 = -.5f*h;
	const float y1 = y0 + h;

	const float u0 = circle_sprite_->u0;
	const float u1 = circle_sprite_->u1;

	const float v0 = circle_sprite_->v0;
	const float v1 = circle_sprite_->v1;

	const float ds = .5f;

	const float s0 = -.01f*show_tics_;
	const float s1 = s0 + ds;

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	circle_sprite_->tex->bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	shine_texture_->bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// add RGB
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	// use alpha of first texture
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

	using va_type = ggl::vertex_array_multitexcoord<GLfloat, 2, GLfloat, 2, GLfloat, 2>;
	(va_type {
		{ x0, y0, u0, v1, s0, 0 },
		{ x1, y0, u1, v1, s0 + ds, 0 },
		{ x0, y1, u0, v0, s1, 1 },
		{ x1, y1, u1, v0, s1 + ds, 1 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();

	// text

	assert(text_);
	text_->draw();

	glPopMatrix();
}

void
lives_widget::on_player_respawn(int lives_left)
{
	printf("respawned! %d lives left\n", lives_left);

	initialize_text(lives_left);
	show_tics_ = 240;
}

void
lives_widget::on_player_death()
{
	printf("died!\n");
	show_tics_ = 0;
}

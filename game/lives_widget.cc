#include <cassert>
#include <sstream>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/texture.h>
#include <ggl/resources.h>
#include <ggl/util.h>

#include "game.h"
#include "lives_widget.h"

lives_widget::lives_widget(game& g)
: widget { g }
, show_tics_ { 0 }
, circle_ { new shiny_sprite_quad { ggl::res::get_sprite("lives-left-circle.png"), game_, .5, -.02 } }
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
		ss << lives_left << " LEFT";
	else
		ss << "LAST PLAYER";

	text_.reset(new text_quad { ggl::res::get_font("fonts/tiny.spr"), ss.str() });
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

	vec2f circle_scale, text_pos;
	quad::horiz_align text_ha;
	quad::vert_align text_va;

	if (pos.x < .5f*game_.viewport_width) {
		circle_scale.x = -1.f;
		text_ha = quad::horiz_align::RIGHT;
	} else {
		circle_scale.x = 1.f;
		text_ha = quad::horiz_align::LEFT;
	}

	if (pos.y < .5f*game_.viewport_height) {
		circle_scale.y = -1;
		text_va = quad::vert_align::BOTTOM;
	} else {
		circle_scale.y = 1;
		text_va = quad::vert_align::TOP;
	}

	text_pos = circle_scale*vec2f { -110, -80 };

	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0);

	// circle

	glPushMatrix();
	glScalef(circle_scale.x, circle_scale.y, 1);
	circle_->draw();
	glPopMatrix();

	// text

	assert(text_);

	glPushMatrix();
	glTranslatef(text_pos.x, text_pos.y, 0);
	text_->draw(text_ha, text_va);
	glPopMatrix();

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

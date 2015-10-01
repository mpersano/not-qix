#include <cassert>	
#include <sstream>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/resources.h>

#include "game.h"
#include "lives_widget.h"

lives_widget::lives_widget(game& g)
: widget { g }
, show_tics_ { 0 }
, circle_sprite_ { ggl::res::get_sprite("lives-left-circle.png") }
, arrow_sprite_{ ggl::res::get_sprite("lives-left-arrow.png") }
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
	if (show_tics_) {
		auto pos = game_.get_player_screen_position();

		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);

		// circle

		circle_sprite_->draw(ggl::sprite::horiz_align::CENTER, ggl::sprite::vert_align::CENTER);

		// arrow

		glPushMatrix();

		const float sx = pos.x > .5f*game_.viewport_width ? -1.f : 1.f;
		const float sy = pos.y > .5f*game_.viewport_height ? -1.f : 1.f;
		glScalef(sx, sy, 1);

		arrow_sprite_->draw(ggl::sprite::horiz_align::CENTER, ggl::sprite::vert_align::CENTER);
		glPopMatrix();

		// text

		assert(text_);
		text_->draw();

		glPopMatrix();
	}
}

void
lives_widget::on_player_respawn(int lives_left)
{
	printf("respawned! %d lives left\n", lives_left);

	initialize_text(lives_left);
	show_tics_ = 120;
}

void
lives_widget::on_player_death()
{
	printf("died!\n");
	show_tics_ = 0;
}

#include <ggl/gl.h>

#include "debug_gfx.h"
#include "game.h"
#include "lives_widget.h"

lives_widget::lives_widget(game& g)
: widget { g }
, show_tics_ { 0 }
{
	auto& p = game_.get_player();

	respawn_conn_ =
		p.get_respawn_event().connect(
			std::bind(&lives_widget::on_player_respawn, this, std::placeholders::_1));

	death_conn_ =
		p.get_death_event().connect(
			std::bind(&lives_widget::on_player_death, this));
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
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		glColor4f(1, 0, 0, 1);

		draw_circle(game_.get_player_screen_position(), 30);
	}
}

void
lives_widget::on_player_respawn(int lives_left)
{
	printf("respawned! %d lives left\n", lives_left);
	show_tics_ = 120;
}

void
lives_widget::on_player_death()
{
	printf("died!\n");
	show_tics_ = 0;
}

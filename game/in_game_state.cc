#include <ggl/core.h>

#include "level.h"
#include "in_game_state.h"

namespace {
const int MARGIN = 8;
}

in_game_state::in_game_state(int width, int height)
: app_state { width, height }
, game_ { width - 2*MARGIN, height - 2*MARGIN }
{
	game_.reset(g_levels[0].get());
}

void
in_game_state::draw() const
{
	glPushMatrix();
	glTranslatef(MARGIN, MARGIN, 0);

	game_.draw();

	glPopMatrix();
}

void
in_game_state::update(float dt)
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

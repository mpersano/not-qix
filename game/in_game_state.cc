#include <ggl/core.h>

#include "level.h"
#include "game_app.h"
#include "in_game_state.h"

in_game_state::in_game_state(game_app& app)
: app_state { app }
, game_ { static_cast<int>(app_.get_scene_width()), static_cast<int>(app_.get_scene_height()), true } // UGH
{
	game_.reset(g_levels[0].get());
}

void
in_game_state::draw() const
{
	game_.draw();
}

void
in_game_state::update()
{
	game_.update();
}

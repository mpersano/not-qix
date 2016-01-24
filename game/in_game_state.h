#pragma once

#include <list>
#include <memory>

#include <ggl/event.h>

#include "app_state.h"
#include "game.h"

class in_game_state : public app_state
{
public:
	in_game_state(game_app& app);

	void draw() const override;
	void update() override;

private:
	game game_;
};

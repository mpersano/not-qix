#pragma once

#include "app_state.h"
#include "game.h"

class in_game_state : public app_state
{
public:
	in_game_state(int width, int height);

	void draw() const override;
	void update(float dt) override;

private:
	game game_;
	int width_, height_;
};

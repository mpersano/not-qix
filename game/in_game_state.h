#pragma once

#include <list>
#include <memory>

#include "app_state.h"
#include "game.h"

class effect;

class in_game_state : public app_state
{
public:
	in_game_state(int width, int height);

	void draw() const override;
	void update() override;

private:
	void update_game();
	void update_effects();

	game game_;
	std::list<std::unique_ptr<effect>> effects_;
	int width_, height_;
};

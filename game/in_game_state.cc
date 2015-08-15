#include <ggl/core.h>

#include "level.h"
#include "percent_gauge.h"
#include "in_game_state.h"

in_game_state::in_game_state(int width, int height)
: app_state { width, height }
, game_ { width, height }
{
	game_.reset(g_levels[0].get());
	effects_.push_back(std::unique_ptr<effect>(new percent_gauge { game_, height }));
}

void
in_game_state::draw() const
{
	game_.draw();

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

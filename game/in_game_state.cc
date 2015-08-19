#include <ggl/core.h>

#include "level.h"
#include "percent_gauge.h"
#include "in_game_state.h"

in_game_state::in_game_state(int width, int height)
: app_state { width, height }
, game_ { width, height }
, dpad_state_ { 0 }
{
	connect_events();

	game_.reset(g_levels[0].get());

	effects_.push_back(std::unique_ptr<effect>(new percent_gauge { game_, height }));
}

void
in_game_state::connect_events()
{
	using namespace std::placeholders;

	auto *core = ggl::g_core;

	dpad_button_down_conn_ =
		core->get_dpad_button_down_event().connect(
			std::bind(&in_game_state::on_dpad_button_down, this, _1));

	dpad_button_up_conn_ =
		core->get_dpad_button_up_event().connect(
			std::bind(&in_game_state::on_dpad_button_up, this, _1));

	pointer_down_conn_ =
		core->get_pointer_down_event().connect(
			std::bind(&gesture_detector::on_pointer_down, &gesture_detector_, _1, _2));

	pointer_up_conn_ =
		core->get_pointer_up_event().connect(
			std::bind(&gesture_detector::on_pointer_up, &gesture_detector_, _1, _2));

	pointer_motion_conn_ =
		core->get_pointer_motion_event().connect(
			std::bind(&gesture_detector::on_pointer_motion, &gesture_detector_, _1, _2));

	gesture_conn_ =
		gesture_detector_.get_event().connect(
			std::bind(&in_game_state::on_gesture, this, _1));
}

void
in_game_state::draw() const
{
	game_.draw();

	for (auto& p : effects_)
		p->draw();
}

void
in_game_state::update()
{
	update_game();
	update_effects();
}

void
in_game_state::update_game()
{
	if (dpad_state_ & ggl::DPAD_UP)
		game_.move(direction::UP, dpad_state_ & ggl::DPAD_BUTTON1);

	if (dpad_state_ & ggl::DPAD_DOWN)
		game_.move(direction::DOWN, dpad_state_ & ggl::DPAD_BUTTON1);

	if (dpad_state_ & ggl::DPAD_LEFT)
		game_.move(direction::LEFT, dpad_state_ & ggl::DPAD_BUTTON1);

	if (dpad_state_ & ggl::DPAD_RIGHT)
		game_.move(direction::RIGHT, dpad_state_ & ggl::DPAD_BUTTON1);

	game_.update();
}

void
in_game_state::update_effects()
{
	auto it = effects_.begin();

	while (it != effects_.end()) {
		if (!(*it)->update())
			it = effects_.erase(it);
		else
			++it;
	}
}

void
in_game_state::on_dpad_button_down(int button)
{
	dpad_state_ |= button;
}

void
in_game_state::on_dpad_button_up(int button)
{
	dpad_state_ &= ~button;
}

void
in_game_state::on_gesture(gesture g)
{
	printf("gesture! %d\n", static_cast<int>(g));
}

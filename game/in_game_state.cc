#include <ggl/core.h>

#include "level.h"
#include "game_app.h"
#include "in_game_state.h"

in_game_state::in_game_state(game_app& app)
: app_state { app }
, game_ { static_cast<int>(app_.get_scene_width()), static_cast<int>(app_.get_scene_height()) } // UGH
, dpad_state_ { 0u }
{
	connect_events();

	game_.reset(g_levels[0].get());
}

void
in_game_state::connect_events()
{
	using namespace std::placeholders;

	auto core = ggl::g_core;

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
}

void
in_game_state::update()
{
	update_game();
}

void
in_game_state::update_game()
{
	game_.update(dpad_state_);
}

void
in_game_state::on_dpad_button_down(ggl::dpad_button button)
{
	dpad_state_ |= (1u << static_cast<int>(button));
}

void
in_game_state::on_dpad_button_up(ggl::dpad_button button)
{
	dpad_state_ &= ~(1u << static_cast<int>(button));
}

void
in_game_state::on_gesture(gesture g)
{
	switch (g) {
		case gesture::SWIPE_LEFT:
			dpad_state_ = 1u << ggl::LEFT;
			break;

		case gesture::SWIPE_RIGHT:
			dpad_state_ = 1u << ggl::RIGHT;
			break;

		case gesture::SWIPE_UP:
			dpad_state_ = 1u << ggl::DOWN;
			break;

		case gesture::SWIPE_DOWN:
			dpad_state_ = 1u << ggl::UP;
			break;
	}
}

#pragma once

#include <list>
#include <memory>

#include <ggl/dpad_button.h>
#include <ggl/event.h>

#include "gesture_detector.h"
#include "app_state.h"
#include "game.h"

class widget;

class in_game_state : public app_state
{
public:
	in_game_state(int width, int height);

	void draw() const override;
	void update() override;

private:
	void connect_events();

	void update_game();
	void update_widgets();

	void on_dpad_button_down(ggl::dpad_button button);
	void on_dpad_button_up(ggl::dpad_button button);
	void on_gesture(gesture g);

	void on_game_started();

	game game_;

	unsigned dpad_state_;

	std::list<std::unique_ptr<widget>> widgets_;

	ggl::event_connection_ptr dpad_button_down_conn_;
	ggl::event_connection_ptr dpad_button_up_conn_;

	ggl::event_connection_ptr pointer_down_conn_;
	ggl::event_connection_ptr pointer_up_conn_;
	ggl::event_connection_ptr pointer_motion_conn_;

	ggl::event_connection_ptr gesture_conn_;

	ggl::event_connection_ptr game_started_conn_;

	gesture_detector gesture_detector_;
};

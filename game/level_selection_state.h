#pragma once

#include <ggl/dpad_button.h>
#include <ggl/event.h>

#include "app_state.h"

class level_selection_state : public app_state
{
public:
	level_selection_state(game_app& app);

	void draw() const override;
	void update() override;

private:
	void connect_events();

	void on_dpad_button_down(ggl::dpad_button button);
	void on_dpad_button_up(ggl::dpad_button button);

	ggl::event_connection_ptr dpad_button_down_conn_;
	ggl::event_connection_ptr dpad_button_up_conn_;
};

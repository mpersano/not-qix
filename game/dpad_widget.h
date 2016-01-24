#pragma once

#include <ggl/event.h>
#include <ggl/core.h>
#include <ggl/vec2.h>

#include "widget.h"

class dpad_widget : public widget
{
public:
	dpad_widget(game& g);

	bool update() override;
	void draw() const override;

	ggl::connectable_event<ggl::core::dpad_button_event_handler>& get_dpad_button_down_event();
	ggl::connectable_event<ggl::core::dpad_button_event_handler>& get_dpad_button_up_event();

private:
	void connect_events();

	void on_pointer_down(int pointer_id, float x, float y);
	void on_pointer_up(int pointer_id);
	void on_pointer_motion(int pointer_id, float x, float y);

	vec2f to_viewport_coords(float x, float y) const;

	void set_stick_pos(const vec2f& p);

	void update_dpad_state();

	vec2f stick_center_;
	vec2f button_center_;
	vec2f stick_pos_;

	int stick_pointer_id_;
	int button_pointer_id_;

	unsigned dpad_state_;

	ggl::event_connection_ptr pointer_down_conn_;
	ggl::event_connection_ptr pointer_up_conn_;
	ggl::event_connection_ptr pointer_motion_conn_;

	ggl::event<ggl::core::dpad_button_event_handler> dpad_button_down_event_;
	ggl::event<ggl::core::dpad_button_event_handler> dpad_button_up_event_;
};

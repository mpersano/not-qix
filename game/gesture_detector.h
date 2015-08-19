#pragma once

#include <deque>

#include <ggl/event.h>

enum class gesture { SWIPE_LEFT, SWIPE_RIGHT, SWIPE_UP, SWIPE_DOWN, NONE };

class gesture_detector
{
public:
	void on_pointer_down(int x, int y);
	void on_pointer_up(int x, int y);
	void on_pointer_motion(int x, int y);

	using event_handler = std::function<void(gesture)>;
	ggl::connectable_event<event_handler>& get_event();

private:
	gesture find_gesture() const;

	struct pointer_state {
		float when;
		int x, y;
	};

	std::deque<pointer_state> states_;
	ggl::event<event_handler> event_;
};

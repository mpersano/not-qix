#pragma once

#include <ggl/event.h>
#include <ggl/noncopyable.h>

class effect : private ggl::noncopyable
{
public:
	virtual ~effect() = default;

	bool update();
	virtual void draw() const = 0;

	virtual bool is_position_absolute() const = 0;

	using finished_event_handler = std::function<void(void)>;
	ggl::connectable_event<finished_event_handler>& get_finished_event();

private:
	virtual bool do_update() = 0;

	ggl::event<finished_event_handler> finished_event_;
};

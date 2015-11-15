#pragma once

#include <memory>

#include <ggl/event.h>
#include <ggl/app.h>
#include <ggl/dpad_button.h>

namespace ggl {

class asset;

class core
{
public:
	core(app& a);
	virtual ~core();

	virtual void run() = 0;

	virtual int get_viewport_width() const = 0;
	virtual int get_viewport_height() const = 0;

	virtual std::unique_ptr<asset> get_asset(const std::string& path) const = 0;

	virtual float now() const = 0;

	using dpad_button_event_handler = std::function<void(dpad_button)>;
	using pointer_event_handler = std::function<void(int, int)>;

	connectable_event<dpad_button_event_handler>& get_dpad_button_down_event();
	connectable_event<dpad_button_event_handler>& get_dpad_button_up_event();

	connectable_event<pointer_event_handler>& get_pointer_down_event();
	connectable_event<pointer_event_handler>& get_pointer_up_event();
	connectable_event<pointer_event_handler>& get_pointer_motion_event();

protected:
	app& app_;

	event<dpad_button_event_handler> dpad_button_down_event_;
	event<dpad_button_event_handler> dpad_button_up_event_;

	event<pointer_event_handler> pointer_down_event_;
	event<pointer_event_handler> pointer_up_event_;
	event<pointer_event_handler> pointer_motion_event_;
};

extern core *g_core;

}

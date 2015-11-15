#include <ggl/core.h>

namespace ggl {

core *g_core;

core::core(app& a)
: app_ { a }
{ }

core::~core() = default;

connectable_event<core::dpad_button_event_handler>&
core::get_dpad_button_down_event()
{
	return dpad_button_down_event_;
}

connectable_event<core::dpad_button_event_handler>&
core::get_dpad_button_up_event()
{
	return dpad_button_up_event_;
}

connectable_event<core::pointer_event_handler>&
core::get_pointer_down_event()
{
	return pointer_down_event_;
}

connectable_event<core::pointer_event_handler>&
core::get_pointer_up_event()
{
	return pointer_up_event_;
}

connectable_event<core::pointer_event_handler>&
core::get_pointer_motion_event()
{
	return pointer_motion_event_;
}

}

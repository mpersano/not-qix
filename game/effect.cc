#include "effect.h"

ggl::connectable_event<effect::finished_event_handler>&
effect::get_finished_event()
{
	return finished_event_;
}

bool
effect::update()
{
	bool rv = do_update();
	if (!rv)
		finished_event_.notify();
	return rv;
}

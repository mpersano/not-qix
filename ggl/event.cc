#include "event.h"

namespace ggl {

event_connection::event_connection(disconnectable_event *e)
: event_ { e }
{ }

event_connection::~event_connection()
{
	if (event_)
		event_->disconnect(this);
}

void
event_connection::set_event(disconnectable_event *e)
{
	event_ = e;
}

}

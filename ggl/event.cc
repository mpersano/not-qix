#include "event.h"

namespace ggl {

event_connection::event_connection(disconnectable_event& owner)
: owner_ { owner }
{ }

event_connection::~event_connection()
{
	owner_.disconnect(this);
}

}

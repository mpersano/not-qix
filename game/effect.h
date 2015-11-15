#pragma once

#include <ggl/event.h>
#include <ggl/noncopyable.h>

namespace ggl {
class sprite_batch;
};

class effect : private ggl::noncopyable
{
public:
	virtual ~effect() = default;

	bool update();
	virtual void draw(ggl::sprite_batch& sb) const = 0;

	virtual bool is_position_absolute() const = 0;

	using finished_event_handler = std::function<void(void)>;
	ggl::connectable_event<finished_event_handler>& get_finished_event();

protected:
	virtual bool do_update() = 0;

	ggl::event<finished_event_handler> finished_event_;
};

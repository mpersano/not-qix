#include <ggl/core.h>

#include "gesture_detector.h"

namespace {

const float MAX_SWIPE_DT = .5;
const int MIN_SWIPE_DIST = 50;

} // (anonymous namespace)

void
gesture_detector::on_pointer_down(int x, int y)
{
	states_.clear();
	states_.push_back({ ggl::g_core->now(), x, y });
}

void
gesture_detector::on_pointer_motion(int x, int y)
{
	auto t = ggl::g_core->now();

	// delete old events
	while (!states_.empty() && (t - states_.front().when) > MAX_SWIPE_DT)
		states_.pop_front();

	states_.push_back({ t, x, y });

	gesture g = find_gesture();

	if (g != gesture::NONE) {
		states_.clear();
		event_.notify(g);
	}
}

void
gesture_detector::on_pointer_up(int x, int y)
{
	states_.clear();
}

gesture
gesture_detector::find_gesture() const
{
	if (states_.size() < 2)
		return gesture::NONE;

	auto distance = [](const pointer_state& e0, const pointer_state& e1)
		{
			int dx = e1.x - e0.x;
			int dy = e1.y - e0.y;
			return sqrtf(dx*dx + dy*dy);
		};

	auto& last = states_.back();

	for (int i = states_.size() - 1; i >= 0; i--) {
		auto& first = states_[i];

		float dist = distance(first, last);

		if (dist > MIN_SWIPE_DIST) {
			float d = 0;

			for (int j = i; j < states_.size() - 1; j++)
				d += distance(states_[j], states_[j + 1]);

			if (fabs(dist - d) < .1f*dist) {
				// found swipe!

				int dx = last.x - first.x;
				int dy = last.y - first.y;

				if (fabs(dx) > fabs(dy))
					return dx > 0 ? gesture::SWIPE_RIGHT : gesture::SWIPE_LEFT;
				else
					return dy > 0 ? gesture::SWIPE_UP : gesture::SWIPE_DOWN;
			}
		}
	}

	return gesture::NONE;
}

ggl::connectable_event<gesture_detector::event_handler>&
gesture_detector::get_event()
{
	return event_;
}

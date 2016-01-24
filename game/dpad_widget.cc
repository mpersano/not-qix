#include <ggl/core.h>
#include <ggl/render.h>
#include <ggl/log.h>

#include "debuggfx.h"
#include "game.h"
#include "dpad_widget.h"

namespace {

static const int BORDER = 10;

static const int STICK_RADIUS = 80;
static const int BUTTON_RADIUS = 60;
static const int STICK_TIP_RADIUS = 20;

static const int MIN_STICK_DIST = 8;

} // namespace

dpad_widget::dpad_widget(game& g)
: widget { g }
, stick_pos_ { 0.f, 0.f }
, stick_center_ { BORDER + STICK_RADIUS, BORDER + STICK_RADIUS }
, button_center_ { game_.viewport_width - BORDER - BUTTON_RADIUS, BUTTON_RADIUS + BORDER }
, stick_pointer_id_ { -1 }
, button_pointer_id_ { -1 }
, dpad_state_ { 0 }
{
	connect_events();
}

bool
dpad_widget::update()
{
}

void
dpad_widget::draw() const
{
	ggl::render::set_color(stick_pointer_id_ != -1 ? ggl::rgba { 1, 1, 0, 1 } : ggl::rgba { 1, 1, 1, .5 });
	draw_circle(stick_center_, STICK_RADIUS, 5.f);

	ggl::render::set_color(ggl::rgba { 1, 1, 1, .5 });
	draw_circle(stick_center_ + stick_pos_, STICK_TIP_RADIUS, 5.f);

	ggl::render::set_color(button_pointer_id_ != -1 ? ggl::rgba { 1, 1, 0, 1 } : ggl::rgba { 1, 1, 1, .5 });
	draw_circle(button_center_, BUTTON_RADIUS, 5.f);
}

void
dpad_widget::connect_events()
{
	using namespace std::placeholders;

	auto core = ggl::g_core;

	pointer_down_conn_ =
		core->get_pointer_down_event().connect(
			std::bind(&dpad_widget::on_pointer_down, this, _1, _2, _3));

	pointer_up_conn_ =
		core->get_pointer_up_event().connect(
			std::bind(&dpad_widget::on_pointer_up, this, _1));

	pointer_motion_conn_ =
		core->get_pointer_motion_event().connect(
			std::bind(&dpad_widget::on_pointer_motion, this, _1, _2, _3));
}

void
dpad_widget::on_pointer_down(int pointer_id, float x, float y)
{
	vec2f p = to_viewport_coords(x, y);

	if (p.x < 2*(BORDER + STICK_RADIUS) && p.y < 2*(BORDER + STICK_RADIUS)) {
		stick_pointer_id_ = pointer_id;
		set_stick_pos(p);
		update_dpad_state();
	} else if (p.x > game_.viewport_width - 2*(BORDER + BUTTON_RADIUS) && p.y < 2*(BORDER + BUTTON_RADIUS)) {
		button_pointer_id_ = pointer_id;
		update_dpad_state();
	}
}

void
dpad_widget::on_pointer_up(int pointer_id)
{
	if (pointer_id == stick_pointer_id_) {
		stick_pointer_id_ = -1;
		stick_pos_ = vec2f { 0, 0 };
		update_dpad_state();
	} else if (pointer_id == button_pointer_id_) {
		button_pointer_id_ = -1;
		update_dpad_state();
	}
}

void
dpad_widget::on_pointer_motion(int pointer_id, float x, float y)
{
	if (pointer_id == stick_pointer_id_) {
		set_stick_pos(to_viewport_coords(x, y));
		update_dpad_state();
	}
}

void
dpad_widget::set_stick_pos(const vec2f& p)
{
	float d = std::min(distance(p, stick_center_), static_cast<float>(STICK_RADIUS));
	stick_pos_ = stick_center_ + d*normalized(p - stick_center_) - stick_center_;
}

vec2f
dpad_widget::to_viewport_coords(float x, float y) const
{
	auto core = ggl::g_core;

	const auto screen_width = core->get_viewport_width();
	const auto screen_height = core->get_viewport_height();

	const auto scene_width = game_.viewport_width;
	const auto scene_height = game_.viewport_height;

	return { x*static_cast<float>(scene_width)/screen_width, scene_height - 1 - y*static_cast<float>(scene_height)/screen_height };
}

void
dpad_widget::update_dpad_state()
{
	unsigned next_dpad_state = 0;

	float l = length(stick_pos_);

	if (l > MIN_STICK_DIST) {
		if (stick_pos_.y > stick_pos_.x) {
			if (stick_pos_.y < -stick_pos_.x)
				next_dpad_state |= (1u << ggl::dpad_button::LEFT);
			else
				next_dpad_state |= (1u << ggl::dpad_button::UP);
		} else {
			if (stick_pos_.y > -stick_pos_.x)
				next_dpad_state |= (1u << ggl::dpad_button::RIGHT);
			else
				next_dpad_state |= (1u << ggl::dpad_button::DOWN);
		}
	}

	if (button_pointer_id_ != -1)
		next_dpad_state |= (1u << ggl::dpad_button::BUTTON1);

	for (int i = 0; i < ggl::dpad_button::NUM_DPAD_BUTTONS; i++) {
		if ((next_dpad_state ^ dpad_state_) & (1u << i)) {
			if (next_dpad_state & (1u << i)) {
				log_info("%d pressed\n", i);
				dpad_button_down_event_.notify(static_cast<ggl::dpad_button>(i));
			} else {
				log_info("%d released\n", i);
				dpad_button_up_event_.notify(static_cast<ggl::dpad_button>(i));
			}
		}
	}

	dpad_state_ = next_dpad_state;
}

ggl::connectable_event<ggl::core::dpad_button_event_handler>&
dpad_widget::get_dpad_button_down_event()
{
	return dpad_button_down_event_;
}

ggl::connectable_event<ggl::core::dpad_button_event_handler>&
dpad_widget::get_dpad_button_up_event()
{
	return dpad_button_up_event_;
}


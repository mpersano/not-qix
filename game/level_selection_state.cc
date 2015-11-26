#include <ggl/vertex_array.h>
#include <ggl/render.h>
#include <ggl/core.h>

#include "game_app.h"
#include "level_selection_state.h"

level_selection_state::level_selection_state(game_app& app)
: app_state { app }
{
	connect_events();
}

void
level_selection_state::draw() const
{
	auto width = app_.get_scene_width();
	auto height = app_.get_scene_height();

	ggl::render::set_viewport({ { 0, 0 }, { width, height } });

	(ggl::vertex_array_flat<GLshort, 2>
		{ { 0, 0 },
		  { 0, static_cast<GLshort>(height) },
		  { static_cast<GLshort>(width), 0 },
		  { static_cast<GLshort>(width), static_cast<GLshort>(height) } }).draw(GL_TRIANGLE_STRIP, ggl::rgba { .5, 1, .5, 1 });
}

void
level_selection_state::update()
{ }

void
level_selection_state::connect_events()
{
	using namespace std::placeholders;

	auto core = ggl::g_core;

	dpad_button_down_conn_ =
		core->get_dpad_button_down_event().connect(
			std::bind(&level_selection_state::on_dpad_button_down, this, _1));

	dpad_button_up_conn_ =
		core->get_dpad_button_up_event().connect(
			std::bind(&level_selection_state::on_dpad_button_up, this, _1));
}

void
level_selection_state::on_dpad_button_down(ggl::dpad_button button)
{
}

void
level_selection_state::on_dpad_button_up(ggl::dpad_button button)
{
}

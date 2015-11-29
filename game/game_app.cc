#include <cmath>
#include <unistd.h>
#include <sys/time.h>

#include <memory>
#include <functional>
#include <deque>

#include <ggl/gl.h>
#include <ggl/app.h>
#include <ggl/resources.h>

#include "level.h"

#include "script_interface.h"
#include "game_app.h"

#include "in_game_state.h"
#include "level_selection_state.h"
#include "transition_state.h"

namespace {

const float FRAME_INTERVAL = 1.f/60;

} // (anonymous namespace)

game_app::game_app()
{ }

void
game_app::init(unsigned viewport_width, unsigned viewport_height)
{
	update_t_ = 0;

	ggl::res::load_sprite_sheet("sprites/sprites.spr");
	ggl::res::load_programs("shaders/effects.xml");

	init_script_interface();
	init_levels();

	viewport_width_ = viewport_width;
	viewport_height_ = viewport_height;

	scene_width_ = 480;
	scene_height_ = scene_width_*viewport_height_/viewport_width_;

	init_states();

	init_gl_state();
}

void
game_app::init_gl_state()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
}

void
game_app::init_states()
{
	level_selection_state_.reset(new level_selection_state { *this });
	in_game_state_.reset(new in_game_state { *this });
	transition_state_.reset(new transition_state { *this });

#if 1
	cur_state_ = in_game_state_.get();
#else
	cur_state_ = transition_state_.get();

	static_cast<transition_state *>(cur_state_)->reset(level_selection_state_.get(), in_game_state_.get());
#endif
}

void
game_app::update_and_render(float dt)
{
	update_t_ += dt;

	while (update_t_ > FRAME_INTERVAL) {
		cur_state_->update();
		update_t_ -= FRAME_INTERVAL;
	}

	glViewport(0, 0, viewport_width_, viewport_height_);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	cur_state_->draw();
}

void
game_app::start_in_game()
{
	cur_state_ = in_game_state_.get();
}

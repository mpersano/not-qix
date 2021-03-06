#include <ggl/resources.h>
#include <ggl/program.h>
#include <ggl/window.h>
#include <ggl/vertex_array.h>

#include "game_app.h"
#include "transition_state.h"

namespace {
const int TRANSITION_TICS = 120;
}

transition_state::transition_state(game_app& app)
: app_state { app }
, fb_from_ { app_.get_scene_width(), app_.get_scene_height() }
, fb_to_ {  app_.get_scene_width(), app_.get_scene_height() }
, program_ { ggl::res::get_program("transition") }
, prev_state_ { nullptr }
, next_state_ { nullptr }
, tics_ { 0 }
{ }

void
transition_state::reset(const app_state *prev_state, const app_state *next_state)
{
	prev_state_ = prev_state;
	next_state_ = next_state;
}

void
transition_state::draw() const
{
	fb_from_.bind();
	prev_state_->draw();

	fb_to_.bind();
	next_state_->draw();

	ggl::window().bind();

	gl_check(glActiveTexture(GL_TEXTURE0));
	fb_from_.bind_texture();

	gl_check(glActiveTexture(GL_TEXTURE1));
	fb_to_.bind_texture();

	gl_check(glActiveTexture(GL_TEXTURE0));

	program_->use();

	program_->set_uniform_i("from", 0);
	program_->set_uniform_i("to", 1);

	program_->set_uniform_f("level", static_cast<float>(tics_)/TRANSITION_TICS);
	program_->set_uniform_f("resolution", fb_from_.get_width(), fb_from_.get_height());

	glDisable(GL_BLEND);

	(ggl::vertex_array_texcoord<GLshort, 2, GLshort, 2>
	  { { { -1, -1, 0, 0 },
	      { -1,  1, 0, 1 },
	      {  1, -1, 1, 0 },
	      {  1,  1, 1, 1 } } }).draw(GL_TRIANGLE_STRIP);
}

void
transition_state::update()
{
	if (++tics_ == TRANSITION_TICS)
		app_.start_in_game();
}

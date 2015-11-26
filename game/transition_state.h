#pragma once

#include <ggl/framebuffer.h>

#include "app_state.h"

namespace ggl {
class gl_program;
}

class transition_state : public app_state
{
public:
	transition_state(game_app& app);

	void reset(const app_state *prev_state, const app_state *next_state);

	void draw() const override;
	void update() override;

private:
	ggl::framebuffer fb_from_;
	ggl::framebuffer fb_to_;

	const ggl::gl_program *program_;

	const app_state *prev_state_;
	const app_state *next_state_;

	int tics_;
};

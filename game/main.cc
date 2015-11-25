#include <cmath>
#include <unistd.h>
#include <sys/time.h>

#include <memory>
#include <functional>
#include <deque>

#include <ggl/gl.h>
#include <ggl/main.h>
#include <ggl/app.h>
#include <ggl/resources.h>

#include "level.h"
#include "in_game_state.h"
#include "script_interface.h"

namespace {

const float FRAME_INTERVAL = 1.f/60;

} // (anonymous namespace)

class game_app : public ggl::app
{
public:
	game_app();

	void init(int viewport_width, int viewport_height) override;
	void update_and_render(float dt) override;

private:
	void init_gl_state();

	std::unique_ptr<app_state> cur_state_;
	float update_t_;

	int viewport_width_, viewport_height_;
	int scene_width_, scene_height_;
};

game_app::game_app()
{
	init_script_interface();
}

void
game_app::init(int viewport_width, int viewport_height)
{
	viewport_width_ = viewport_width;
	viewport_height_ = viewport_height;

	scene_width_ = 480;
	scene_height_ = scene_width_*viewport_height_/viewport_width_;

	ggl::res::load_sprite_sheet("sprites/sprites.spr");

	init_levels();

	cur_state_.reset(new in_game_state { scene_width_, scene_height_ });

	update_t_ = 0;

	init_gl_state();
}

void
game_app::init_gl_state()
{
	// default state

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);

	// texture environment parameters

	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glActiveTexture(GL_TEXTURE1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// add RGB
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	// use alpha of first texture
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
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

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, scene_width_, 0, scene_height_, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	cur_state_->draw();
}

GGL_MAIN(game_app)

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
	glDisable(GL_DEPTH_TEST);

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

	cur_state_->draw();
}

GGL_MAIN(game_app)

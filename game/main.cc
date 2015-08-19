#include <cmath>
#include <unistd.h>
#include <sys/time.h>

#include <memory>
#include <functional>
#include <deque>

#include <ggl/gl.h>
#include <ggl/main.h>
#include <ggl/app.h>

#include "level.h"
#include "in_game_state.h"

namespace {

const float FRAME_INTERVAL = 1.f/60;

} // (anonymous namespace)

class game_app : public ggl::app
{
public:
	game_app();

	void init(int width, int height) override;
	void update_and_render(float dt) override;

private:
	std::unique_ptr<app_state> cur_state_;
	float update_t_;
	int width_, height_;
};

game_app::game_app()
{ }

void
game_app::init(int width, int height)
{
	width_ = width;
	height_ = height;

	ggl::res::load_sprite_sheet("sprites/sprites");

	init_levels();

	cur_state_.reset(new in_game_state { width, height });

	update_t_ = 0;
}

void
game_app::update_and_render(float dt)
{
	update_t_ += dt;

	while (update_t_ > FRAME_INTERVAL) {
		cur_state_->update();
		update_t_ -= FRAME_INTERVAL;
	}

	glViewport(0, 0, width_, height_);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width_, 0, height_, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	cur_state_->draw();
}

GGL_MAIN(game_app)

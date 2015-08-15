#include <unistd.h>

#include <memory>

#include <ggl/gl.h>
#include <ggl/main.h>
#include <ggl/app.h>

#include "level.h"
#include "in_game_state.h"

namespace {

const float UPDATE_RATE = 1.f/60;
}

class game_app : public ggl::app
{
public:
	void init(int width, int height) override;
	void update_and_render(float dt) override;

private:
	std::unique_ptr<app_state> cur_state_;
	float update_t_;
	int width_, height_;
};

void
game_app::init(int width, int height)
{
	width_ = width;
	height_ = height;

	init_levels();
	cur_state_.reset(new in_game_state { width, height });

	update_t_ = 0;
}

void
game_app::update_and_render(float dt)
{
	update_t_ += dt;

	while (update_t_ > UPDATE_RATE) {
		cur_state_->update();
		update_t_ -= UPDATE_RATE;
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

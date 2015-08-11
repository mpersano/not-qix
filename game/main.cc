#include <unistd.h>

#include <memory>

#include <ggl/gl.h>
#include <ggl/resources.h>
#include <ggl/main.h>
#include <ggl/app.h>

#include "level.h"
#include "game.h"

class game_app : public ggl::app
{
public:
	void init(int width, int height) override;
	void update_and_render(float dt) override;

private:
	static const int MARGIN = 8;

	std::unique_ptr<game> game_;
	std::unique_ptr<level> level_;
	int width_, height_;
};

void
game_app::init(int width, int height)
{
	width_ = width;
	height_ = height;

	level_.reset(new level { "images/girl.png", "images/girl-mask.png" });
	game_.reset(new game { width_ - 2*MARGIN, height_ - 2*MARGIN });

	game_->reset(level_.get());
}

void
game_app::update_and_render(float dt)
{
	glViewport(0, 0, width_, height_);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width_, 0, height_, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	unsigned dpad_state = ggl::g_core->get_dpad_state();

	if (dpad_state & ggl::DPAD_UP)
		game_->move(direction::UP, dpad_state & ggl::DPAD_BUTTON1);

	if (dpad_state & ggl::DPAD_DOWN)
		game_->move(direction::DOWN, dpad_state & ggl::DPAD_BUTTON1);

	if (dpad_state & ggl::DPAD_LEFT)
		game_->move(direction::LEFT, dpad_state & ggl::DPAD_BUTTON1);

	if (dpad_state & ggl::DPAD_RIGHT)
		game_->move(direction::RIGHT, dpad_state & ggl::DPAD_BUTTON1);

	game_->update(dt);

	glPushMatrix();
	glTranslatef(MARGIN, MARGIN, 0);
	game_->draw();
	glPopMatrix();
}

GGL_MAIN(game_app)

#if 0
#if defined(ANDROID)
void
android_main(android_app *state)
{
	app_dummy();

	ggl::res::init();

	game_app(state).run();
}
#else
int
main(int argc, char *argv[])
{
	chdir("data");

	ggl::res::init();

	game_app(320, 480).run();
}
#endif
#endif

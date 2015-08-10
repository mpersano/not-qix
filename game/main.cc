#include <GL/glew.h>

#include <ggl/window.h>

#include "resources.h"
#include "level.h"
#include "game.h"

class game_window : public ggl::window
{
public:
	game_window(int width, int height);

	void draw(float dt) override;

private:
	static const int MARGIN = 8;

	game game_;
	level level_; // XXX for now
};

game_window::game_window(int width, int height)
: ggl::window { width, height, "game", }
, game_ { width - 2*MARGIN, height - 2*MARGIN }
, level_ { "data/images/girl.png", "data/images/girl-mask.png" }
{
	game_.reset(&level_);
}

void
game_window::draw(float dt)
{
	glViewport(0, 0, width_, height_);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width_, 0, height_, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (dpad_state_ & ggl::DPAD_UP)
		game_.move(direction::UP, dpad_state_ & ggl::DPAD_BUTTON1);

	if (dpad_state_ & ggl::DPAD_DOWN)
		game_.move(direction::DOWN, dpad_state_ & ggl::DPAD_BUTTON1);

	if (dpad_state_ & ggl::DPAD_LEFT)
		game_.move(direction::LEFT, dpad_state_ & ggl::DPAD_BUTTON1);

	if (dpad_state_ & ggl::DPAD_RIGHT)
		game_.move(direction::RIGHT, dpad_state_ & ggl::DPAD_BUTTON1);

	game_.update(dt);

	glPushMatrix();
	glTranslatef(MARGIN, MARGIN, 0);
	game_.draw();
	glPopMatrix();
}

int
main(int argc, char *argv[])
{
	gp::res::init();

	game_window g(320, 480);
	g.run();
}

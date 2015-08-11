#include <unistd.h>

#include <ggl/gl.h>
#include <ggl/resources.h>

#include "level.h"
#include "game.h"

#if defined(ANDROID)
#include <ggl/android/window.h>
using abstract_window = ggl::android::window;
#else
#include <ggl/sdl/window.h>
using abstract_window = ggl::sdl::window;
#endif

class game_window : public abstract_window
{
public:
#if defined(ANDROID)
	game_window(android_app *state);
#else
	game_window(int width, int height);
#endif

	void update_and_render(float dt) override;

private:
	static const int MARGIN = 8;

	game game_;
	level level_; // XXX for now
};

// PLEASE MAKE ME LESS UGLY

#if defined(ANDROID)
game_window::game_window(android_app *state)
: abstract_window { state }
, game_ { 320 - 2*MARGIN, 480 - 2*MARGIN }
#else
game_window::game_window(int width, int height)
: abstract_window { width, height, "game", }
, game_ { width - 2*MARGIN, height - 2*MARGIN }
#endif
, level_ { "images/girl.png", "images/girl-mask.png" }
{
	game_.reset(&level_);
}

void
game_window::update_and_render(float dt)
{
	glViewport(0, 0, width_, height_);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, width_, 0, height_, -1, 1);

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

#if defined(ANDROID)
void
android_main(android_app *state)
{
	app_dummy();

	ggl::res::init();

	game_window(state).run();
}
#else
int
main(int argc, char *argv[])
{
	chdir("data");

	ggl::res::init();

	game_window(320, 480).run();
}
#endif

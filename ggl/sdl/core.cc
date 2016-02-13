#include <ggl/asset.h>
#include <ggl/panic.h>

#include <ggl/sdl/asset.h>
#include <ggl/sdl/core.h>
#include <ggl/sdl/audio_player.h>

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cerrno>

#include <SDL.h>
#include <GL/glew.h>
#include <physfs.h>

namespace ggl { namespace sdl {

core::core(app& a, int width, int height, const char *caption, bool fullscreen)
: ggl::core { a }
, width_ { width }
, height_ { height }
{
	// SDL

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		panic("SDL_Init: %s", SDL_GetError());

	Uint32 flags = SDL_OPENGL;

	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	if (SDL_SetVideoMode(width, height, 0, flags) == 0)
		panic("SDL_SetVideoMode: %s", SDL_GetError());

	SDL_WM_SetCaption(caption, NULL);

	// GLEW

	if (GLenum rv = glewInit())
		panic("glewInit: %s", glewGetErrorString(rv));

	// OpenAL

	if (!(al_device_ = alcOpenDevice(nullptr)))
		panic("alcOpenDevice failed");

	if (!(al_context_ = alcCreateContext(al_device_, nullptr)))
		panic("alcCreateContext failed");

	alcMakeContextCurrent(al_context_);
	alGetError();

	// PhysFS

	if (!PHYSFS_init(nullptr))
		panic("PHYSFS_init: %s", PHYSFS_getLastError());

	PHYSFS_mount(".", nullptr, 1);
	PHYSFS_mount("assets.zip", nullptr, 1);
}

core::~core()
{
	// OpenAL

	alcMakeContextCurrent(nullptr);
	alcDestroyContext(al_context_);
	alcCloseDevice(al_device_);

	// PhysFS

	PHYSFS_deinit();

	// SDL

	SDL_Quit();
}

void
core::run()
{
	init_resources();

	app_.init(width_, height_);

	float last_update = now();

	for (;;) {
		float t = now();
		app_.update_and_render(t - last_update);
		last_update = t;

		SDL_GL_SwapBuffers();

		if (!poll_events())
			break;
	}
}

bool
core::poll_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return false;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					return false;
				on_key_down(event.key.keysym.sym);
				break;

			case SDL_KEYUP:
				on_key_up(event.key.keysym.sym);
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
					pointer_down_event_.notify(0, event.button.x, event.button.y);
				break;

			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
					pointer_up_event_.notify(0);
				break;

			case SDL_MOUSEMOTION:
				if (event.motion.state & SDL_BUTTON_LMASK)
					pointer_motion_event_.notify(0, event.motion.x, event.motion.y);
				break;
		}
	}

	return true;
}

void
core::on_key_down(int keysym)
{
	switch (keysym) {
		case SDLK_UP:
			dpad_button_down_event_.notify(dpad_button::UP);
			break;

		case SDLK_DOWN:
			dpad_button_down_event_.notify(dpad_button::DOWN);
			break;

		case SDLK_LEFT:
			dpad_button_down_event_.notify(dpad_button::LEFT);
			break;

		case SDLK_RIGHT:
			dpad_button_down_event_.notify(dpad_button::RIGHT);
			break;

		case SDLK_LCTRL:
			dpad_button_down_event_.notify(dpad_button::BUTTON1);
			break;

		case SDLK_SPACE:
			dpad_button_down_event_.notify(dpad_button::BUTTON2);
			break;
	}
}

void
core::on_key_up(int keysym)
{
	switch (keysym) {
		case SDLK_UP:
			dpad_button_up_event_.notify(dpad_button::UP);
			break;

		case SDLK_DOWN:
			dpad_button_up_event_.notify(dpad_button::DOWN);
			break;

		case SDLK_LEFT:
			dpad_button_up_event_.notify(dpad_button::LEFT);
			break;

		case SDLK_RIGHT:
			dpad_button_up_event_.notify(dpad_button::RIGHT);
			break;

		case SDLK_LCTRL:
			dpad_button_up_event_.notify(dpad_button::BUTTON1);
			break;

		case SDLK_SPACE:
			dpad_button_up_event_.notify(dpad_button::BUTTON2);
			break;
	}
}

std::unique_ptr<ggl::asset>
core::get_asset(const std::string& path) const
{
	return std::unique_ptr<ggl::asset>(new asset(path));
}

std::unique_ptr<ggl::audio_player>
core::get_audio_player() const
{
	return std::unique_ptr<ggl::audio_player>(new ggl::oal::audio_player());
}

float
core::now() const
{
	return 1e-3f*SDL_GetTicks();
}

} }

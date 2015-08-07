#include "window.h"
#include "panic.h"

#include <cstdio>
#include <cstdarg>

#include <SDL.h>
#include <GL/glew.h>

namespace {

float
now()
{
	return .001*SDL_GetTicks();
}

}

namespace ggl {

window::window(int width, int height, const char *caption, bool fullscreen)
: width_(width)
, height_(height)
, dpad_state_(0)
, dump_frames_(false)
, frame_num_(0)
, frame_data_(3*width*height)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		panic("SDL_Init: %s", SDL_GetError());

	Uint32 flags = SDL_OPENGL;

	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	if (SDL_SetVideoMode(width, height, 0, flags) == 0)
		panic("SDL_SetVideoMode: %s", SDL_GetError());

	SDL_WM_SetCaption(caption, NULL);

	if (GLenum rv = glewInit())
		panic("glewInit: %s", glewGetErrorString(rv));
}

window::~window()
{
	SDL_Quit();
}

void
window::run()
{
	float last_update = now();

	for (;;) {
		float t = now();
		draw(t - last_update);
		last_update = t;

		if (dump_frames_) {
			static int n = 0;
			if ((n%3) == 0) {
				dump_frame();
			}
			++n;
		}

		SDL_GL_SwapBuffers();

		if (!poll_events())
			break;
	}
}

bool
window::poll_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return false;

			case SDL_MOUSEBUTTONDOWN:
				on_mouse_button_down(event.button.button, event.button.x, event.button.y);
				break;

			case SDL_MOUSEBUTTONUP:
				on_mouse_button_up(event.button.button);
				break;

			case SDL_MOUSEMOTION:
				on_mouse_motion(event.motion.x, event.motion.y);
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					return false;
				} else {
					on_key_down(event.key.keysym.sym);
				}
				break;

			case SDL_KEYUP:
				on_key_up(event.key.keysym.sym);
				break;
		}
	}

	return true;
}

void
window::on_key_down(int keysym)
{
	switch (keysym) {
		case SDLK_UP:
			dpad_state_ |= DPAD_UP;
			break;

		case SDLK_DOWN:
			dpad_state_ |= DPAD_DOWN;
			break;

		case SDLK_LEFT:
			dpad_state_ |= DPAD_LEFT;
			break;

		case SDLK_RIGHT:
			dpad_state_ |= DPAD_RIGHT;
			break;

		case SDLK_LCTRL:
			dpad_state_ |= DPAD_BUTTON1;
			break;

		case SDLK_SPACE:
			dpad_state_ |= DPAD_BUTTON2;
			break;
	}
}

void
window::on_key_up(int keysym)
{
	switch (keysym) {
		case SDLK_UP:
			dpad_state_ &= ~DPAD_UP;
			break;

		case SDLK_DOWN:
			dpad_state_ &= ~DPAD_DOWN;
			break;

		case SDLK_LEFT:
			dpad_state_ &= ~DPAD_LEFT;
			break;

		case SDLK_RIGHT:
			dpad_state_ &= ~DPAD_RIGHT;
			break;

		case SDLK_LCTRL:
			dpad_state_ &= ~DPAD_BUTTON1;
			break;

		case SDLK_SPACE:
			dpad_state_ &= ~DPAD_BUTTON2;
			break;
	}
}

void
window::dump_frame()
{
	glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, &frame_data_[0]); 

	char path[80];
	sprintf(path, "%05d.ppm", frame_num_);

	if (FILE *out = fopen(path, "wb")) {
		fprintf(out, "P6\n%d %d\n255\n", width_, height_);
		fwrite(&frame_data_[0], frame_data_.size(), 1, out);
		fclose(out);

		++frame_num_;
	}
}

void
window::set_dump_frames(bool b)
{
	dump_frames_ = b;
}

}

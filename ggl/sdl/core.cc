#include <ggl/asset.h>
#include <ggl/panic.h>

#include <ggl/sdl/core.h>

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cerrno>

#include <SDL.h>
#include <GL/glew.h>

namespace ggl { namespace sdl {

class asset : public ggl::asset
{
public:
	asset(const std::string& path)
	: stream_(fopen(path.c_str(), "rb"))
	{
		if (!stream_)
			panic("failed to open `%s': %s", path.c_str(), strerror(errno));
	}

	~asset()
	{
		fclose(stream_);
	}

	size_t read(void *buf, size_t size) override;
	uint8_t read_uint8() override;
	uint16_t read_uint16() override;
	uint32_t read_uint32() override;

private:
	FILE *stream_;
};

uint8_t
asset::read_uint8()
{
	uint8_t v;

	if (read(&v, 1) != 1) {
		panic("read failed: %s", strerror(errno));
	}

	return v;
}

uint16_t
asset::read_uint16()
{
	uint16_t lo = static_cast<uint16_t>(read_uint8());
	uint16_t hi = static_cast<uint16_t>(read_uint8());
	return lo | (hi << 8);
}

uint32_t
asset::read_uint32()
{
	uint32_t lo = static_cast<uint32_t>(read_uint16());
	uint32_t hi = static_cast<uint32_t>(read_uint16());
	return lo | (hi << 16);
}

size_t
asset::read(void *buf, size_t size)
{
	return ::fread(buf, 1, size, stream_);
}

core::core(app& a, int width, int height, const char *caption, bool fullscreen)
: ggl::core { a }
, width_ { width }
, height_ { height }
, dpad_state_ { 0 }
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

core::~core()
{
	SDL_Quit();
}

void
core::run()
{
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
		}
	}

	return true;
}

void
core::on_key_down(int keysym)
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
core::on_key_up(int keysym)
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

std::unique_ptr<ggl::asset>
core::get_asset(const std::string& path) const
{
	return std::unique_ptr<ggl::asset>(new asset(path));
}

float
core::now() const
{
	return 1e-3f*SDL_GetTicks();
}

} }

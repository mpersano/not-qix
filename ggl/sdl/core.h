#include <ggl/core.h>

namespace ggl { namespace sdl {

class core : public ggl::core
{
public:
	core(app& a, int width, int height, const char *caption, bool fullscreen);
	virtual ~core();

	void run();

	int get_viewport_width() const
	{ return width_; }

	int get_viewport_height() const
	{ return height_; }

	unsigned get_dpad_state() const
	{ return dpad_state_; }

	float now() const;

private:
	bool poll_events();
	void on_key_down(int keysym);
	void on_key_up(int keysym);

	int width_, height_;
	unsigned dpad_state_;
};

} }

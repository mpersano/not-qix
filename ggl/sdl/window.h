#include <ggl/window.h>

namespace ggl { namespace sdl {

class window : public ggl::window
{
public:
	window(int width, int height, const char *caption, bool fullscreen = false);
	virtual ~window();

	void run();

	virtual void update_and_render(float dt) = 0;

private:
	bool poll_events();
	void on_key_down(int keysym);
	void dump_frame();
	void on_key_up(int keysym);
};

} }

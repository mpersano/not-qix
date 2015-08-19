#include <ggl/core.h>

namespace ggl { namespace sdl {

class core : public ggl::core
{
public:
	core(app& a, int width, int height, const char *caption, bool fullscreen);
	virtual ~core();

	void run() override;

	int get_viewport_width() const override
	{ return width_; }

	int get_viewport_height() const override
	{ return height_; }

	std::unique_ptr<ggl::asset> get_asset(const std::string& path) const override;

	float now() const override;

private:
	bool poll_events();

	void on_key_down(int keysym);
	void on_key_up(int keysym);

	int width_, height_;
};

} }

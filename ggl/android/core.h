#pragma once

#include <EGL/egl.h>
#include <android_native_app_glue.h>

#include <ggl/core.h>

namespace ggl { namespace android {

class core : public ggl::core
{
public:
	core(app& a, android_app *state);
	~core();

	void run() override;

	int get_viewport_width() const override
	{ return width_; }

	int get_viewport_height() const override
	{ return height_; }

	unsigned get_dpad_state() const override
	{ return 0; }

	std::unique_ptr<ggl::asset> get_asset(const std::string& path) const override;

	float now() const override;

private:
	static int32_t handle_input(android_app *app, AInputEvent *event);
	int32_t handle_input(AInputEvent *event);

	static void handle_cmd(android_app *app, int32_t cmd);
	void handle_cmd(int32_t cmd);

	bool init_display();
	void term_display();

	android_app *state_;

	EGLDisplay display_;
	EGLSurface surface_;
	EGLContext context_;

	int width_, height_;
};

} }

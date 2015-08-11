#pragma once

#include <EGL/egl.h>
#include <android_native_app_glue.h>

#include <ggl/window.h>

namespace ggl { namespace android {

class window : public ggl::window
{
public:
	window(android_app *app);
	~window();

	window(const window&) = delete;
	window& operator=(const window&) = delete;

	void run();

private:
	static int32_t handle_input(android_app *app, AInputEvent *event);
	int32_t handle_input(AInputEvent *event);

	static void handle_cmd(android_app *app, int32_t cmd);
	void handle_cmd(int32_t cmd);

	bool init_display();
	void term_display();

	android_app *app_;

	EGLDisplay display_;
	EGLSurface surface_;
	EGLContext context_;
};

} }

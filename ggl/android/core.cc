#include <time.h>

#include <ggl/log.h>
#include <ggl/gl.h>
#include <ggl/resources.h>
#include <ggl/android/asset.h>
#include <ggl/android/core.h>

namespace ggl { namespace android {

core::core(app& a, android_app *state)
: ggl::core { a }
, state_ { state }
, app_initialized_ { false }
, display_ { EGL_NO_DISPLAY }
, surface_ { EGL_NO_SURFACE }
, context_ { EGL_NO_CONTEXT }
{
	state->userData = this;
	state->onAppCmd = handle_cmd;
	state->onInputEvent = handle_input;
}

core::~core()
{
	term_display();
}

bool
core::init_display()
{
	//
	//   initialize surface
	//

	display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display_, 0, 0);

	const EGLint attribs[] = {
		// EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // request ES2.0
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};

	EGLConfig config;

	EGLint num_configs;
	eglChooseConfig(display_, attribs, &config, 1, &num_configs);

	if (!num_configs) {
		log_error("unable to retrieve EGL config");
		return false;
	}

	EGLint format;
	eglGetConfigAttrib(display_, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(state_->window, 0, 0, format);

	surface_ = eglCreateWindowSurface(display_, config, state_->window, nullptr);

	//
	//   initialize context
	//

	const EGLint context_attribs[] = {
			// EGL_CONTEXT_CLIENT_VERSION, 2, // request ES2.0
			EGL_NONE };
	context_ = eglCreateContext(display_, config, NULL, context_attribs);

	if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_FALSE) {
		log_error("eglMakeCurrent failed");
		return false;
	}

	eglQuerySurface(display_, surface_, EGL_WIDTH, &width_);
	eglQuerySurface(display_, surface_, EGL_HEIGHT, &height_);

	log_info("initialized surface: %dx%d / OpenGL version: %s",
		width_, height_,
		reinterpret_cast<const char *>(glGetString(GL_VERSION)));

	return true;
}

void
core::term_display()
{
	if (display_ != EGL_NO_DISPLAY) {
		eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (context_ != EGL_NO_CONTEXT)
			eglDestroyContext(display_, context_);

		if (surface_ != EGL_NO_SURFACE)
			eglDestroySurface(display_, surface_);

		eglTerminate(display_);
	}

	display_ = EGL_NO_DISPLAY;
	context_ = EGL_NO_CONTEXT;
	surface_ = EGL_NO_SURFACE;
}

int32_t
core::handle_input(struct android_app *app, AInputEvent *event)
{
	return static_cast<core *>(app->userData)->handle_input(event);
}

int32_t
core::handle_input(AInputEvent *event)
{
	switch (AInputEvent_getType(event)) {
		case AINPUT_EVENT_TYPE_MOTION:
			{
			const int x = static_cast<int>(AMotionEvent_getX(event, 0));
			const int y = static_cast<int>(AMotionEvent_getY(event, 0));

			switch (AMotionEvent_getAction(event)) {
				case AMOTION_EVENT_ACTION_DOWN:
					pointer_down_event_.notify(x, y);
					return 1;

				case AMOTION_EVENT_ACTION_UP:
					pointer_up_event_.notify(x, y);
					return 1;

				case AMOTION_EVENT_ACTION_MOVE:
					pointer_motion_event_.notify(x, y);
					return 1;
			}
			}
			break;

		case AINPUT_EVENT_TYPE_KEY:
			if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
				switch (AKeyEvent_getKeyCode(event)) {
					case AKEYCODE_MENU:
						// XXX something here
						return 1;

					case AKEYCODE_BACK:
						// XXX something here
						return 1;
				}
			}
			break;
	}

	return 0;
}

void
core::handle_cmd(struct android_app* app, int32_t cmd)
{
	static_cast<core *>(app->userData)->handle_cmd(cmd);
}

void
core::handle_cmd(int32_t cmd)
{
	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.
			log_info("APP_CMD_SAVE_STATE");
			break;

		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			log_info("APP_CMD_INIT_WINDOW");
			if (state_->window != NULL) {
				init_display();
				res::load_gl_resources();

				if (!app_initialized_) {
					app_.init(width_, height_);
					app_initialized_ = true;
				}
			}
			break;

		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			log_info("APP_CMD_TERM_WINDOW");

			res::unload_gl_resources();
			term_display();
			break;

		case APP_CMD_GAINED_FOCUS:
			log_info("APP_CMD_GAINED_FOCUS");
			break;

		case APP_CMD_LOST_FOCUS:
			log_info("APP_CMD_LOST_FOCUS");
			break;
	}
}

void
core::run()
{
	float last_update = 0;

	bool running = true;

	while (running) {
		int ident;
		int events;
		android_poll_source *source;

		while ((ident = ALooper_pollAll(0, NULL, &events, reinterpret_cast<void **>(&source))) >= 0) {
			if (source)
				source->process(state_, source);

			if (state_->destroyRequested) {
				log_info("destroy requested");
				running = false;
				break;
			}
		}

		if (!running)
			break;

		if (display_ != EGL_NO_DISPLAY) {
			float t = now();
			if (last_update == 0)
				last_update = t;
			app_.update_and_render(t - last_update);
			last_update = t;

			eglSwapBuffers(display_, surface_);
		}
	}

	term_display();
}

std::unique_ptr<ggl::asset>
core::get_asset(const std::string& path) const
{
	return std::unique_ptr<ggl::asset>(new asset(state_->activity->assetManager, path));
}

float
core::now() const
{
	struct timespec tp;
	static time_t secbase;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	if (!secbase)
		secbase = tp.tv_sec;

	return (tp.tv_sec - secbase) + 1e-9f*tp.tv_nsec;
}

} }

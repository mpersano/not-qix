#define GGL_MAIN(APP_CLASS) \
void android_main(android_app *state) \
{ \
	app_dummy(); \
	ggl::res::init(); \
	APP_CLASS app; \
	ggl::g_core = new ggl::android::core(app, state); \
	ggl::g_core->run(); \
}

#define GGL_MAIN(APP_CLASS) \
int main(int argc, char *argv[]) \
{ \
	chdir("data"); \
	ggl::res::init(); \
	APP_CLASS app; \
	ggl::g_core = new ggl::sdl::core(app, 640, 426, "test", false); \
	ggl::g_core->run(); \
}

#define GGL_MAIN(APP_CLASS) \
int main(int argc, char *argv[]) \
{ \
	APP_CLASS app; \
	chdir("data"); \
	ggl::res::init(); \
	ggl::g_core = new ggl::sdl::core(app, 320, 480, "test", false); \
	ggl::g_core->run(); \
}

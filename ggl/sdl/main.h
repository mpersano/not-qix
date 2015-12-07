#define GGL_MAIN(APP_CLASS) \
int main(int argc, char *argv[]) \
{ \
	chdir("data"); \
	APP_CLASS app; \
	ggl::g_core = new ggl::sdl::core(app, 800, 480, "test", false); \
	ggl::g_core->run(); \
}

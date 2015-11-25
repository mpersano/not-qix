#pragma once

#include <ggl/panic.h>

#if 0
#define gl_check(expr) \
	[&] { \
		expr; \
		auto e = glGetError(); \
		if (e != GL_NO_ERROR) \
			panic("%s:%d: GL error: %x", __FILE__, __LINE__, e); \
	}()

#define gl_check_r(expr) \
	[&] { \
		auto r = expr; \
		auto e = glGetError(); \
		if (e != GL_NO_ERROR) \
			panic("%s:%d: GL error: %x", __FILE__, __LINE__, e); \
		return r; \
	}()
#else
#define gl_check(expr) expr
#define gl_check_r(expr) expr
#endif

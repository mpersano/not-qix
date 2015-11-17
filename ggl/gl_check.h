#pragma once

#include <ggl/panic.h>

#define gl_check(expr) \
	do { \
		expr; \
		auto e = glGetError(); \
		if (e != GL_NO_ERROR) \
			panic("GL error: %s:%d: %x", __FILE__, __LINE__, e); \
	} while (false);

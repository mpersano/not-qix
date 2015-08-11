#pragma once

#if defined(ANDROID)
#include <GLES/gl.h>
#define glOrtho glOrthof
#elif defined(_WIN32)
#include <GL/wglew.h>
#else
#include <GL/glew.h>
#endif

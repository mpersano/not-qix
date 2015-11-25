#pragma once

#if defined(ANDROID)
#include <GLES3/gl3.h>
#elif defined(_WIN32)
#include <GL/wglew.h>
#else
#include <GL/glew.h>
#endif

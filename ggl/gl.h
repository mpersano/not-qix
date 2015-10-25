#pragma once

#if defined(ANDROID)
#include <GLES/gl.h>
#define glOrtho glOrthof
#define GL_SOURCE0_RGB GL_SRC0_RGB
#define GL_SOURCE1_RGB GL_SRC1_RGB
#define GL_SOURCE2_RGB GL_SRC2_RGB
#define GL_SOURCE0_ALPHA GL_SRC0_ALPHA
#define GL_SOURCE1_ALPHA GL_SRC1_ALPHA
#define GL_SOURCE2_ALPHA GL_SRC2_ALPHA
#elif defined(_WIN32)
#include <GL/wglew.h>
#else
#include <GL/glew.h>
#endif

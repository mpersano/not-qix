#pragma once

#if defined(ANDROID)
#include <ggl/android/core.h>
#include <ggl/android/main.h>
#else
#include <unistd.h>
#include <ggl/sdl/core.h>
#include <ggl/sdl/main.h>
#endif

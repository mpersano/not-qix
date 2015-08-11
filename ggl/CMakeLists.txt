if (ANDROID)
	include_directories(
		"${ANDROID_NDK}/sources/android/native_app_glue")
else()
	find_package(SDL REQUIRED)
	find_package(GLEW REQUIRED)

	include_directories(
		${SDL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR})
endif()

include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/libpng)

set(GGL_SOURCES
	panic.cc
	core.cc
	asset.cc
	xwchar.cc
	texture.cc
	font.cc
	image.cc
	resources.cc)

if (ANDROID)
	list(APPEND GGL_SOURCES
		"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
		android/core.cc)
else()
	list(APPEND GGL_SOURCES
		sdl/asset.cc
		sdl/core.cc)
endif()

add_library(ggl ${GGL_SOURCES})

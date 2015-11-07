find_package(ZLIB REQUIRED)

include_directories(
	${ZLIB_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
	${TINYXML_INCLUDE_DIR})

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

set(GGL_SOURCES
	panic.cc
	event.cc
	core.cc
	texture.cc
	font.cc
	image.cc
	sprite.cc
	action.cc
	resources.cc
	sprite_manager.cc
	vec2_util.cc)

if (ANDROID)
	list(APPEND GGL_SOURCES
		"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
		android/asset.cc
		android/core.cc)
else()
	list(APPEND GGL_SOURCES
		sdl/asset.cc
		sdl/core.cc)
endif()

add_library(ggl ${GGL_SOURCES})

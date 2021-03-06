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
	find_package(PhysFS REQUIRED)
	find_package(OpenAL REQUIRED)
	find_package(OggVorbis REQUIRED)

	include_directories(
		${SDL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR}
		${PhysFS_INCLUDE_DIR}
		${OPENAL_INCLUDE_DIR}
		${VORBIS_INCLUDE_DIR}
		${OGG_INCLUDE_DIR})
endif()

set(GGL_SOURCES
	panic.cc
	event.cc
	core.cc
	asset.cc
	texture.cc
	font.cc
	image.cc
	mesh.cc
	sprite.cc
	action.cc
	tween.cc
	resources.cc
	render.cc
	sprite_manager.cc
	program_manager.cc
	gl_buffer.cc
	gl_vertex_array.cc
	program.cc
	framebuffer.cc
	window.cc
	vec2_util.cc)

if (ANDROID)
	list(APPEND GGL_SOURCES
		"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
		android/asset.cc
		android/core.cc)
else()
	list(APPEND GGL_SOURCES
		sdl/asset.cc
		sdl/core.cc
		sdl/audio_player.cc)
endif()

add_library(ggl ${GGL_SOURCES})

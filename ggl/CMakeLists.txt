if (ANDROID)
	# XXX: stuff here
else()
	find_package(SDL REQUIRED)
	find_package(GLEW REQUIRED)
	include_directories(${SDL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})
endif()

include_directories(${CMAKE_SOURCE_DIR}/libpng)

set(GGL_SOURCES
	panic.cc
	window.cc
	xwchar.cc
	texture.cc
	font.cc
	image.cc
	file.cc
	resources.cc)

if (ANDROID)
	# XXX: stuff here
else()
	list(APPEND GGL_SOURCES sdl/window.cc)
endif()

add_library(ggl ${GGL_SOURCES})

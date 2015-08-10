set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(SDL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PNG REQUIRED)

include_directories(${SDL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})

set(GGL_SOURCES
	xwchar.cc
	window.cc
	panic.cc
	texture.cc
	font.cc
	image.cc
	file.cc
	resources.cc)

add_library(ggl ${GGL_SOURCES})

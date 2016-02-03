cmake_minimum_required(VERSION 2.6)

project(intro)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_definitions(-DGLEW_STATIC -U__STRICT_ANSI__)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif ()

add_subdirectory(external/libpng)
add_subdirectory(external/tinyxml)
add_subdirectory(external/lua5.2)

add_subdirectory(assets)
add_subdirectory(ggl)
add_subdirectory(game)

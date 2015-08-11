cmake_minimum_required(VERSION 2.6)

project(intro)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(libpng)
add_subdirectory(ggl)
add_subdirectory(game)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(SDL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)

include_directories(
	${SDL_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR})

link_directories(${CMAKE_BINARY_DIR}/ggl)

set(INTRO_SOURCES
	level.cc
	game.cc
	main.cc)

add_executable(game ${INTRO_SOURCES})

target_link_libraries(
	game
	ggl
	${SDL_LIBRARY}
	${GLEW_LIBRARY}
	${OPENGL_LIBRARIES}
	${PNG_LIBRARIES})

set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)

add_custom_command(TARGET game POST_BUILD
	COMMAND mkdir -p ${DATA_DIR})

add_custom_command(TARGET game POST_BUILD
	COMMAND cp -r ${CMAKE_SOURCE_DIR}/assets/images ${DATA_DIR})

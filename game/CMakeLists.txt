if (ANDROID)
	# XXX stuff here
else()
	find_package(SDL REQUIRED)
	find_package(GLEW REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(ZLIB REQUIRED)

	include_directories(
		${ZLIB_INCLUDE_DIR}
		${SDL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR})

	set(GAME_LIBRARIES
		${ZLIB_LIBRARIES}
		${SDL_LIBRARY}
		${GLEW_LIBRARY}
		${OPENGL_LIBRARIES})
endif()

include_directories(
	${CMAKE_SOURCE_DIR}/libpng)

link_directories(
	${CMAKE_BINARY_DIR}/ggl
	${CMAKE_BINARY_DIR}/png)

set(INTRO_SOURCES
	level.cc
	game.cc
	main.cc)

add_executable(game ${INTRO_SOURCES})

target_link_libraries(
	game
	ggl
	png
	${GAME_LIBRARIES})

if (NOT ANDROID)
	set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
	
	add_custom_command(TARGET game POST_BUILD
		COMMAND mkdir -p ${DATA_DIR})
	
	add_custom_command(TARGET game POST_BUILD
		COMMAND cp -r ${CMAKE_SOURCE_DIR}/assets/images ${DATA_DIR})

	add_custom_command(TARGET game POST_BUILD
		COMMAND cp -r ${CMAKE_SOURCE_DIR}/assets/fonts ${DATA_DIR})
endif()

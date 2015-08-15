find_package(ZLIB REQUIRED)

include_directories(
	${ZLIB_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
	${TINYXML_INCLUDE_DIR})

link_directories(
	${CMAKE_BINARY_DIR}/ggl
	${CMAKE_BINARY_DIR}/png)

set(GAME_LIBRARIES
	ggl
	${PNG_LIBRARY}
	${ZLIB_LIBRARIES}
	${TINYXML_LIBRARY})

if (ANDROID)
	include_directories(
		"${ANDROID_NDK}/sources/android/native_app_glue")

	find_library(LOG_LIBRARY NAMES log PATHS /usr/lib)
	find_library(ANDROID_LIBRARY NAMES android PATHS /usr/lib)
	find_library(GLES_LIBRARY NAMES GLESv1_CM PATHS /usr/lib)
	find_library(EGL_LIBRARY NAMES EGL PATHS /usr/lib)

	list(APPEND GAME_LIBRARIES
		${LOG_LIBRARY}
		${ANDROID_LIBRARY}
		${GLES_LIBRARY}
		${EGL_LIBRARY})
else()
	find_package(SDL REQUIRED)
	find_package(GLEW REQUIRED)
	find_package(OpenGL REQUIRED)

	include_directories(
		${SDL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR})

	list(APPEND GAME_LIBRARIES
		${SDL_LIBRARY}
		${GLEW_LIBRARY}
		${OPENGL_LIBRARIES})
endif()

set(GAME_SOURCES
	main.cc
	level.cc
	game.cc
	in_game_state.cc
	percent_gauge.cc)

if (ANDROID)
	list(APPEND GAME_SOURCES
		"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
	add_library(game SHARED ${GAME_SOURCES})
else()
	add_executable(game ${GAME_SOURCES})
endif()

target_link_libraries(game ${GAME_LIBRARIES})

if (NOT ANDROID)
	set(DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")
	set(FONT_DIR "${DATA_DIR}/fonts")
	set(PACKFONT_COMMAND "${CMAKE_BINARY_DIR}/tools/packsprites/packfont")
	
	add_custom_command(TARGET game POST_BUILD
		COMMAND mkdir -p ${DATA_DIR})
	
	add_custom_command(TARGET game POST_BUILD
		COMMAND cp -r ${CMAKE_SOURCE_DIR}/assets/images ${DATA_DIR})

	add_custom_command(TARGET game POST_BUILD
		COMMAND cp -r ${CMAKE_SOURCE_DIR}/assets/data ${DATA_DIR})

	add_custom_command(TARGET game POST_BUILD
		COMMAND mkdir -p ${FONT_DIR})

	add_custom_command(TARGET game POST_BUILD
		COMMAND ${PACKFONT_COMMAND} -w 256 -h 256 -s 20 -g 2 -1 ${CMAKE_SOURCE_DIR}/assets/fonts/font.ttf small x20-x7e
		WORKING_DIRECTORY ${FONT_DIR}
		DEPENDS ${PACKFONT_COMMAND})

	add_custom_command(TARGET game POST_BUILD
		COMMAND ${PACKFONT_COMMAND} -w 256 -h 256 -s 12 -g 2 -1 ${CMAKE_SOURCE_DIR}/assets/fonts/font.ttf tiny x20-x7e
		WORKING_DIRECTORY ${FONT_DIR}
		DEPENDS ${PACKFONT_COMMAND})
endif()

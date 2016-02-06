find_package(ZLIB REQUIRED)

include_directories(
	${ZLIB_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
	${TINYXML_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/external)

link_directories(
	${CMAKE_BINARY_DIR}/ggl
	${CMAKE_BINARY_DIR}/png)

set(GAME_LIBRARIES
	ggl
	${PNG_LIBRARY}
	${ZLIB_LIBRARIES}
	${TINYXML_LIBRARY}
	${LUA52_LIBRARY})

if (ANDROID)
	include_directories(
		"${ANDROID_NDK}/sources/android/native_app_glue")

	find_library(LOG_LIBRARY NAMES log PATHS /usr/lib)
	find_library(ANDROID_LIBRARY NAMES android PATHS /usr/lib)
	find_library(GLES_LIBRARY NAMES GLESv3 PATHS /usr/lib)
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
	find_package(PhysFS REQUIRED)

	include_directories(
		${SDL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR}
		${PhysFS_INCLUDE_DIR})

	list(APPEND GAME_LIBRARIES
		${SDL_LIBRARY}
		${GLEW_LIBRARY}
		${OPENGL_LIBRARIES}
		${PhysFS_LIBRARY})
endif()

set(GAME_SOURCES
	main.cc
	game_app.cc
	level.cc
	shiny_sprite.cc
	game.cc
	post_filter.cc
	player.cc
	script_interface.cc
	effect.cc
	entity.cc
	foe.cc
	boss.cc
	debuggfx.cc
	miniboss.cc
	powerup.cc
	explosion.cc
	particles.cc
	app_state.cc
	level_selection_state.cc
	in_game_state.cc
	transition_state.cc
	widget.cc
	lives_widget.cc
	percent_widget.cc
	dpad_widget.cc)

set(ASSET_DIR "${CMAKE_BINARY_DIR}/assets/assets")

if (ANDROID)
	set(DEST_ASSETS "${CMAKE_SOURCE_DIR}/android/assets")

	add_custom_command(
		OUTPUT ${DEST_ASSETS}
		COMMAND ln -s ${ASSET_DIR} "${CMAKE_SOURCE_DIR}/android/assets"
		DEPENDS ${ASSET_DIR})

	add_library(
		game SHARED
		${GAME_SOURCES}
		"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
		${DEST_ASSETS})
else()
	set(DEST_ASSETS "${CMAKE_CURRENT_BINARY_DIR}/assets.zip")

	add_custom_command(
		OUTPUT ${DEST_ASSETS}
		COMMAND zip -r ${DEST_ASSETS} .
		DEPENDS ${ASSET_DIR}
		WORKING_DIRECTORY ${ASSET_DIR})

	add_executable(
		game
		${GAME_SOURCES}
		${DEST_ASSETS})
endif()

target_link_libraries(game ${GAME_LIBRARIES})

if (ANDROID)
	add_custom_command(TARGET game POST_BUILD
		COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/android/libs/armeabi/
		COMMAND cp -r ${CMAKE_CURRENT_BINARY_DIR}/libgame.so ${CMAKE_SOURCE_DIR}/android/libs/armeabi/)
endif()

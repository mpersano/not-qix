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
	gesture_detector.cc
	level.cc
	tween.cc
	shiny_sprite.cc
	game.cc
	player.cc
	script_interface.cc
	effect.cc
	entity.cc
	foe.cc
	boss.cc
	miniboss.cc
	powerup.cc
	explosion.cc
	particles.cc
	in_game_state.cc
	widget.cc
	lives_widget.cc
	percent_widget.cc)

set(ASSET_DIR "${CMAKE_BINARY_DIR}/assets/assets")

if (ANDROID)
	set(DEST_ASSET_DIR "${CMAKE_SOURCE_DIR}/android/assets")
else()
	set(DEST_ASSET_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")
endif()

add_custom_command(
	OUTPUT ${DEST_ASSET_DIR}
	COMMAND ln -s ${ASSET_DIR} ${DEST_ASSET_DIR}
	DEPENDS ${ASSET_DIR})

if (ANDROID)
	add_library(
		game SHARED
		${GAME_SOURCES}
		"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
		${DEST_ASSET_DIR})
else()
	add_executable(
		game
		${GAME_SOURCES}
		${DEST_ASSET_DIR})
endif()

target_link_libraries(game ${GAME_LIBRARIES})

if (ANDROID)
	add_custom_command(TARGET game POST_BUILD
		COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/android/libs/armeabi/
		COMMAND cp -r ${CMAKE_CURRENT_BINARY_DIR}/libgame.so ${CMAKE_SOURCE_DIR}/android/libs/armeabi/)
endif()

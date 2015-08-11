find_package(ZLIB REQUIRED)

include_directories(
	${ZLIB_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/libpng)

link_directories(
	${CMAKE_BINARY_DIR}/ggl
	${CMAKE_BINARY_DIR}/png)

set(GAME_LIBRARIES
	${ZLIB_LIBRARIES}
	ggl
	png)

if (ANDROID)
	include_directories(
		"${ANDROID_NDK}/sources/android/native_app_glue")

	list(APPEND GAME_LIBRARIES
		log
		android
		GLESv1_CM
		EGL)
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
	level.cc
	game.cc
	main.cc)

if (ANDROID)
	list(APPEND GAME_SOURCES
		"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
	add_library(game SHARED ${GAME_SOURCES})
else()
	add_executable(game ${GAME_SOURCES})
endif()

target_link_libraries(game ${GAME_LIBRARIES})

if (NOT ANDROID)
	set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
	
	add_custom_command(TARGET game POST_BUILD
		COMMAND mkdir -p ${DATA_DIR})
	
	add_custom_command(TARGET game POST_BUILD
		COMMAND cp -r ${CMAKE_SOURCE_DIR}/assets/images ${DATA_DIR})

	add_custom_command(TARGET game POST_BUILD
		COMMAND cp -r ${CMAKE_SOURCE_DIR}/assets/fonts ${DATA_DIR})
endif()

find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)

include_directories(
	${PNG_INCLUDE_DIRS}
	${FREETYPE_INCLUDE_DIRS})

set(COMMON_SOURCES
	file_writer.cc
	font.cc
	panic.cc
	pixmap.cc
	sprite.cc
	sprite_base.cc
	sprite_packer.cc)

add_executable(packfont packfont.cc ${COMMON_SOURCES})

target_link_libraries(
	packfont
	${FREETYPE_LIBRARIES}
	${PNG_LIBRARIES})

add_executable(packsprites packfont.cc ${COMMON_SOURCES})

target_link_libraries(
	packsprites
	${FREETYPE_LIBRARIES}
	${PNG_LIBRARIES})

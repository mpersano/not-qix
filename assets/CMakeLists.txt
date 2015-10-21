set(ASSET_DIR assets)

# scripts/images/data

foreach(NAME scripts images data)
	add_custom_command(
		OUTPUT ${ASSET_DIR}/${NAME}
		COMMAND mkdir -p ${ASSET_DIR}
		COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/${NAME} ${ASSET_DIR}/${NAME})

	list(APPEND DEST_FILES ${ASSET_DIR}/${NAME})
endforeach()

# fonts 

set(FONT_DIR "${ASSET_DIR}/fonts")

add_custom_command(
	OUTPUT ${FONT_DIR}
	COMMAND mkdir -p ${FONT_DIR})

add_custom_command(
	OUTPUT ${FONT_DIR}/small.png
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/packfonts.sh
	WORKING_DIRECTORY ${FONT_DIR}
	DEPENDS ${FONT_DIR})

# FIXME this is wrong.
#
# On the first build, mtime(${FONT_DIR}) will be updated, because new files
# were added to the directory. But since the ${FONT_DIR}/small.png target
# depends on ${FONT_DIR}, on the next build `make` will needlessly run the
# command again because a dependency - ${FONT_DIR} - is newer than the target.

list(APPEND DEST_FILES ${FONT_DIR}/small.png)

# sprites

set(SPRITE_DIR "${ASSET_DIR}/sprites")

add_custom_command(
	OUTPUT ${SPRITE_DIR}
	COMMAND mkdir -p ${SPRITE_DIR})

add_custom_command(
	OUTPUT ${SPRITE_DIR}/sprites.png
	COMMAND packsprites -w 1024 -h 1024 -t sprites sprites ${CMAKE_CURRENT_SOURCE_DIR}/sprites
	WORKING_DIRECTORY ${SPRITE_DIR}
	DEPENDS ${SPRITE_DIR})

# FIXME it would be nice to add the actual list of sprites to the dependency list, so sprites
# would be repacked when we update something.

list(APPEND DEST_FILES ${SPRITE_DIR}/sprites.png)

add_custom_target(
	genassets ALL
	DEPENDS ${DEST_FILES})

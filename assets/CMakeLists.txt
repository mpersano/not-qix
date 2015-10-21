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
	OUTPUT ${FONT_DIR}/small.png
	COMMAND mkdir -p ${FONT_DIR}
	COMMAND cd ${FONT_DIR} && ${CMAKE_CURRENT_SOURCE_DIR}/packfonts.sh)

list(APPEND DEST_FILES ${FONT_DIR}/small.png)

# sprites

set(SPRITE_DIR "${ASSET_DIR}/sprites")

add_custom_command(
	OUTPUT ${SPRITE_DIR}/sprites.png
	COMMAND mkdir -p ${SPRITE_DIR}
	COMMAND cd ${SPRITE_DIR} && packsprites -w 1024 -h 1024 -t sprites sprites ${CMAKE_CURRENT_SOURCE_DIR}/sprites)

# FIXME it would be nice to add the actual list of sprites to the dependency list, so sprites
# would be repacked when we update something.

list(APPEND DEST_FILES ${SPRITE_DIR}/sprites.png)

add_custom_target(
	genassets ALL
	DEPENDS ${DEST_FILES})

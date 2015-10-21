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
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/packfonts.sh WORKING_DIRECTORY ${FONT_DIR}
	DEPENDS ${FONT_DIR})

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

list(APPEND DEST_FILES ${SPRITE_DIR}/sprites.png)

add_custom_target(
	genassets ALL
	DEPENDS ${DEST_FILES})

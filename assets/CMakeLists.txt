set(ASSET_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")

# scripts/images/animations/shaders/data

foreach(NAME scripts images animations shaders data music)
	add_custom_command(
		OUTPUT ${ASSET_DIR}/${NAME}
		COMMAND mkdir -p ${ASSET_DIR}
		COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/${NAME} ${ASSET_DIR}/${NAME})

	list(APPEND DEST_FILES ${ASSET_DIR}/${NAME})
endforeach()

# fonts 

set(FONT_DIR "${ASSET_DIR}/fonts")

set(FONTS ${FONT_DIR}/hud-big.0.png ${FONT_DIR}/hud-small.0.png)

add_custom_command(
	OUTPUT ${FONTS}
	COMMAND mkdir -p ${FONT_DIR}
	COMMAND cd ${FONT_DIR} && ${CMAKE_CURRENT_SOURCE_DIR}/packfonts.sh
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/packfonts.sh)

list(APPEND DEST_FILES ${FONTS})

# meshes

set(MESH_DIR "${ASSET_DIR}/meshes")
set(MESHES boss miniboss pod)

set(PACKMESH ${CMAKE_CURRENT_SOURCE_DIR}/packmesh.pl)

foreach(MESH_NAME ${MESHES})
	set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/meshes/${MESH_NAME})

	set(OBJ ${SRC}.obj)
	set(MTL ${SRC}.mtl)

	set(MESH ${MESH_DIR}/${MESH_NAME}.msh)

	add_custom_command(
		OUTPUT ${MESH}
		COMMAND mkdir -p ${MESH_DIR}
		COMMAND cd ${MESH_DIR} && ${PACKMESH} ${SRC} > ${MESH}
		DEPENDS ${PACKMESH} ${OBJ} ${MTL})

	list(APPEND DEST_FILES ${MESH})
endforeach()

# sprites

set(GENSPRITE_DIR "${CMAKE_CURRENT_BINARY_DIR}/gensprites")

add_custom_command(
	OUTPUT ${GENSPRITE_DIR}/player-core-00.png
	COMMAND mkdir -p ${GENSPRITE_DIR}
	COMMAND cd ${GENSPRITE_DIR} && ${CMAKE_CURRENT_SOURCE_DIR}/gensprites.sh
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gensprites.sh)

set(SPRITE_DIR "${ASSET_DIR}/sprites")

set(SPRITESHEETS ${SPRITE_DIR}/sprites.0.png)

add_custom_command(
	OUTPUT ${SPRITESHEETS}
	COMMAND mkdir -p ${SPRITE_DIR}
	COMMAND cd ${SPRITE_DIR} && packsprites -w 1024 -h 1024 -t sprites sprites ${CMAKE_CURRENT_SOURCE_DIR}/sprites ${GENSPRITE_DIR}
	DEPENDS ${GENSPRITE_DIR}/player-core-00.png)

# FIXME it would be nice to add the actual list of sprites to the dependency list, so sprites
# would be repacked when we update something.

list(APPEND DEST_FILES ${SPRITESHEETS})

add_custom_target(
	genassets ALL
	DEPENDS ${DEST_FILES})

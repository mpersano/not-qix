FIND_PATH(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
FIND_PATH(OGG_INCLUDE_DIR ogg/ogg.h)

FIND_LIBRARY(VORBIS_LIBRARY NAMES vorbis)
FIND_LIBRARY(OGG_LIBRARY NAMES ogg)
FIND_LIBRARY(VORBISFILE_LIBRARY NAMES vorbisfile)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	OggVorbis REQUIRED_VARS
	VORBIS_INCLUDE_DIR
	OGG_INCLUDE_DIR
	VORBIS_LIBRARY
	OGG_LIBRARY
	VORBISFILE_LIBRARY)
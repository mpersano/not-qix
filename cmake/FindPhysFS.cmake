FIND_PATH(PhysFS_INCLUDE_DIR physfs.h)
FIND_LIBRARY(PhysFS_LIBRARY NAMES physfs)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	PhysFS REQUIRED_VARS
	PhysFS_INCLUDE_DIR
	PhysFS_LIBRARY)
set(LUA52_SOURCES
	lapi.c
	lcode.c
	lctype.c
	ldebug.c
	ldo.c
	ldump.c
	lfunc.c
	lgc.c
	llex.c
	lmem.c
	lobject.c
	lopcodes.c
	lparser.c
	lstate.c
	lstring.c
	ltable.c
	ltm.c
	lundump.c
	lvm.c
	lzio.c
	lauxlib.c
	lbaselib.c
	lbitlib.c
	lcorolib.c
	ldblib.c
	liolib.c
	lmathlib.c
	loslib.c
	lstrlib.c
	ltablib.c
	loadlib.c
	linit.c)

add_definitions(-DLUA_COMPAT_ALL)

add_library(lua5.2 ${LUA52_SOURCES})

set(LUA52_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(LUA52_LIBRARY lua5.2 PARENT_SCOPE)

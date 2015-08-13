set(TINYXML_SOURCES
	tinystr.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp)

add_library(tinyxml ${TINYXML_SOURCES})

set(TINYXML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(TINYXML_LIBRARY tinyxml PARENT_SCOPE)

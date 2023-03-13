cmake_minimum_required (VERSION 3.12)

set (PROJECT Minefield)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set (BUILD_TYPE LIB_RELEASE)
	set (BUILD_NAME_POSTFIX)
else()
	set (BUILD_TYPE LIB_DEBUG)
	set (BUILD_NAME_POSTFIX _dbg)
endif()

# Build name
set (BUILD_NAME Minefield${BUILD_NAME_POSTFIX})

# libEngine path
set (LIBENGINE_PATH ../../libEngine)

# Where libs library should be
if (WIN32)
	set (LIBS_PATH ${CMAKE_SOURCE_DIR}/${LIBENGINE_PATH}/Libraries/Win/libs${BUILD_NAME_POSTFIX})
else()
	set (LIBS_PATH ${CMAKE_SOURCE_DIR}/${LIBENGINE_PATH}/Libraries/Linux/libs${BUILD_NAME_POSTFIX})
endif()

# Where the source code should be
set (SOURCE_DIR ..)

# Include source file list
set (SOURCE_GLOBBING_LIST ${SOURCE_DIR}/*.cpp)

# Directories/files that we don't want to include
set (EXCLUDE_SOURCE ${SOURCE_DIR}/Build/)

# where we are building to
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/${SOURCE_DIR}/Minefield)

if (WIN32)
	set (GUI_TYPE WIN32)
	set (RESOURCES ${SOURCE_DIR}/Resources/Main.rc)
elseif (APPLE)
	SET (GUI_TYPE MACOSX_BUNDLE)
	set (RESOURCES)
endif()

project (${PROJECT})

file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS ${SOURCE_GLOBBING_LIST})

# Excludes unwanted source dirs
foreach(SOURCE_PATH ${SOURCE_LIST})
	foreach(EXCLUDE_PATH ${EXCLUDE_SOURCE})
		string (FIND ${SOURCE_PATH} ${EXCLUDE_PATH} EXCLUDE_DIR_FOUND)
		if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
		list (REMOVE_ITEM SOURCE_LIST ${SOURCE_PATH})
		endif ()
	endforeach()
endforeach()

if (MSVC)
	add_definitions(/D${BUILD_TYPE} /W3)
else()
	add_definitions(-D${BUILD_TYPE})
endif()

add_definitions("-std=c++20")
add_executable (${BUILD_NAME} ${GUI_TYPE} ${SOURCE_LIST} ${RESOURCES})

# Win32
if (WIN32)
	if (MSVC)
		target_link_libraries(${BUILD_NAME} ${MOUNT_LIBS}.lib user32.lib)
	else()
		target_link_libraries(${BUILD_NAME} ${MOUNT_LIBS}.a user32.a)
	endif()
else()
	target_link_libraries(${BUILD_NAME} ${LIBS_PATH}.a SDL2 dl)
endif()


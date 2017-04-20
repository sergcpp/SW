cmake_minimum_required(VERSION 2.8.4)
project(SWrasterizer)

set(SDL_STATIC ON)

add_definitions(-DGLEW_STATIC)
include_directories(libs/glm libs/glew/include)

add_definitions(-DUSE_SW_RENDER)

if(WIN32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(SDL2_INCLUDE_DIRS libs/SDL2/i686-w64-mingw32/include)
		set(SDL2_LIBRARIES "libs/SDL2/i686-w64-mingw32/lib")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(SDL2_LIBRARIES "libs/SDL2/lib/x86")
		set(SDL2_INCLUDE_DIRS libs/SDL2/include)
	endif()
else(WIN32)
    include(FindPkgConfig)
    pkg_search_module (SDL2 REQUIRED sdl2)
endif(WIN32)

include_directories(libs)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(.)

link_directories(${SDL2_LIBRARIES})

add_subdirectory(SW)
add_subdirectory(the_game)
cmake_minimum_required(VERSION 3.2)
project(test_ui)

IF(WIN32)
    IF(SDL_STATIC)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(LIBS :libSDL2main.a :libSDL2.a :libSDL2_mixer.a winmm imm32 version)
		elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set(LIBS SDL2main.lib SDL2.lib winmm imm32 version)
        endif()
    ELSE(SDL_STATIC)
		set(LIBS SDL2)
    ENDIF(SDL_STATIC)
ELSE(WIN32)
    #set(LIBS X11 :libSDL2.a dl pthread GL glut c)
    set(LIBS X11 SDL2 dl pthread c)
ENDIF(WIN32)

add_executable(test_ui main.cpp
                       test_common.h
                       test_renderer.cpp
                       test_widgets.cpp)
target_link_libraries(test_ui ${LIBS} ui)
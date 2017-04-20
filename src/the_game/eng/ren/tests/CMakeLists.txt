cmake_minimum_required(VERSION 3.2)
project(test_ren)

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

set(SOURCE_FILES main.cpp
                 test_anim.cpp
                 test_common.h
                 test_material.cpp
                 test_mesh.cpp
                 test_render_state.cpp
                 test_sparse_array.cpp
                 test_texture.cpp)

if(${RENDERER} STREQUAL "GL")
    set(SOURCE_FILES ${SOURCE_FILES}
                     test_programGL.cpp)
elseif(${RENDERER} STREQUAL "SW")
    set(SOURCE_FILES ${SOURCE_FILES}
                     test_programSW.cpp)
    set(LIBS ${LIBS} SW)
elseif(${RENDERER} STREQUAL "VK")

endif()

add_executable(test_ren ${SOURCE_FILES})
target_link_libraries(test_ren ${LIBS} ren)

cmake_minimum_required(VERSION 2.8.4)
project(ren)

if(WIN32)
    if(SDL_STATIC)
        set(LIBS opengl32)
    else(SDL_STATIC)
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(LIBS opengl32 SDL2 SDL2_mixer)
        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set(LIBS opengl32 SDL2 SDL2_mixer)
        endif()
    endif(SDL_STATIC)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
else(WIN32)
if(${RENDERER} STREQUAL "GL")
    set(LIBS GL)
elseif(${RENDERER} STREQUAL "VK")
    set(LIBS GL)
    add_definitions(-DVK_PROTOTYPES -DVK_USE_PLATFORM_XCB_KHR)
endif()
ENDIF(WIN32)

set(SOURCE_FILES    Anim.h
                    Anim.cpp
                    Camera.h
                    Camera.cpp
                    FrameBuf.h
                    Material.h
                    Material.cpp
                    Matrices_old.h
                    Matrices_old.cpp
                    Mesh.h
                    Mesh.cpp
                    Plane.h
                    Plane.cpp
                    Program.h
                    Program.cpp
                    RenderState.h
                    RenderState.cpp
                    RenderThread.h
                    RenderThread.cpp
                    RingBuffer.h
                    SparseArray.h
                    Texture.h
					Texture.cpp)

if(${RENDERER} STREQUAL "GL")
    set(SOURCE_FILES    ${SOURCE_FILES}
                        FrameBufGL.cpp
						GL.h
                        ProgramGL.cpp
                        RenderStateGL.h
                        TextureGL.cpp)

    set(SOIL_SRC    ../libs/SOIL2/etc1_utils.c 
                    ../libs/SOIL2/image_DXT.c
                    ../libs/SOIL2/image_helper.c
                    ../libs/SOIL2/SOIL2.c)

    set(LIBS ${LIBS} glew SOIL2)
elseif(${RENDERER} STREQUAL "SW")
    set(SOURCE_FILES    ${SOURCE_FILES}
                        RenderStateSW.h
                        TextureSW.cpp)
elseif(${RENDERER} STREQUAL "VK")
    set(SOURCE_FILES    ${SOURCE_FILES}
                        RenderStateVK.cpp
                        RenderStateVK.h)
endif()

add_subdirectory(SW)

list(APPEND ALL_SOURCE_FILES ${SOURCE_FILES})
source_group("src" FILES ${SOURCE_FILES})

add_library(ren STATIC ${ALL_SOURCE_FILES})
target_link_libraries(ren ${LIBS} SW)

add_subdirectory(tests)
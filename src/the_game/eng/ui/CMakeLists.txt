cmake_minimum_required(VERSION 2.8.4)
project(ui)

IF(WIN32)

ELSE(WIN32)
    set(LIBS -pthread)
ENDIF(WIN32)

set(SOURCE_FILES    BaseElement.h
                    BaseElement.cpp
                    BitmapFont.h
                    BitmapFont.cpp
                    ButtonBase.h
                    ButtonBase.cpp
                    ButtonImage.h
                    ButtonImage.cpp
                    ButtonText.h
                    ButtonText.cpp
                    Cursor.h
                    Cursor.cpp
                    EditBox.h
                    EditBox.cpp
                    Frame.h
                    Frame.cpp
                    Image.h
                    Image.cpp
                    LinearLayout.h
                    LinearLayout.cpp
                    Renderer.h
                    TypeMesh.h
                    TypeMesh.cpp
                    Utils.h
                    Utils.cpp
         )

if(${RENDERER} STREQUAL "GL")
    set(SOURCE_FILES    ${SOURCE_FILES}
                        RendererGL.cpp
            )
elseif(${RENDERER} STREQUAL "SW")
    set(SOURCE_FILES    ${SOURCE_FILES}
                        RendererSW.cpp
            )
elseif(${RENDERER} STREQUAL "VK")
    set(SOURCE_FILES    ${SOURCE_FILES}

            )
endif()

list(APPEND ALL_SOURCE_FILES ${SOURCE_FILES})
set_source_files_properties(${SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("src" FILES ${SOURCE_FILES})

list(APPEND ALL_SOURCE_FILES _ui.cpp)
source_group("src" FILES _ui.cpp)

add_library(ui STATIC ${ALL_SOURCE_FILES})
target_link_libraries(ui ren sys ${LIBS})

add_subdirectory(tests)

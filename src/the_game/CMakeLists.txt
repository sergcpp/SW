cmake_minimum_required(VERSION 3.1)
project(the_game)

set(CMAKE_CXX_STANDARD 11)

IF(WIN32)
    IF(SDL_STATIC)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(LIBS :libSDL2main.a :libSDL2.a :libSDL2_mixer.a winmm imm32 version)
		elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set(LIBS SDL2main.lib SDL2.lib winmm imm32 version)
        endif()
    ELSE(SDL_STATIC)
		if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
			set(LIBS opengl32 SDL2 SDL2_mixer)
		elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
			set(LIBS opengl32 SDL2 SDL2_mixer)
		endif()
    ENDIF(SDL_STATIC)
ELSE(WIN32)
    #set(LIBS X11 :libSDL2.a dl pthread GL glut c)
    set(LIBS X11 SDL2 dl pthread GL glut c)
ENDIF(WIN32)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -Wall -Wno-switch -Wno-attributes")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3 -ftree-vectorize -msse -msse2 -msse3 -mssse3 -fvariable-expansion-in-unroller -ftree-vectorizer-verbose=0 -ftree-loop-if-convert-stores -ffast-math")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic-errors -Wall -Wno-switch -Wno-attributes -funroll-loops")
endif()

include_directories(eng)

set(SOURCE_FILES main.cpp
				 Game.h
				 Game.cpp
				 GameApp.h
				 GameApp.cpp)

list(APPEND ALL_SOURCE_FILES ${SOURCE_FILES})
set_source_files_properties(${SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src" FILES ${SOURCE_FILES})

list(APPEND ALL_SOURCE_FILES _game.cpp)
source_group("src" FILES _game.cpp)

set(COMP_SOURCES comp/ISerializable.h
                 comp/Drawable.h
                 comp/Drawable.cpp
				 comp/Transform.h
				 comp/Transform.cpp)

list(APPEND ALL_SOURCE_FILES ${COMP_SOURCES})
set_source_files_properties(${COMP_SOURCES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\comp" FILES ${COMP_SOURCES})

list(APPEND ALL_SOURCE_FILES comp/_comp.cpp)
source_group("src\\comp" FILES comp/_comp.cpp)

set(ENG_SOURCES eng/Config.h
				eng/FlowControl.h
				eng/FlowControl.cpp
				eng/FrameInfo.h
				eng/GameBase.h
				eng/GameBase.cpp
				eng/GameState.h
				eng/GameStateManager.h
				eng/GameStateManager.cpp
				eng/Go.h
				eng/Go.cpp
				eng/GoAlloc.h
				eng/GoComponent.h
				eng/GoID.h
				eng/ObjectPool.h
				eng/Random.h
				eng/Random.cpp
				eng/TimedInput.h
				eng/TimedInput.cpp)

list(APPEND ALL_SOURCE_FILES ${ENG_SOURCES})
set_source_files_properties(${ENG_SOURCES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\eng" FILES ${ENG_SOURCES})

list(APPEND ALL_SOURCE_FILES eng/_eng.cpp)
source_group("src\\eng" FILES eng/_eng.cpp)

set(REN_SOURCES eng/ren/Anim.h
                eng/ren/Anim.cpp
                eng/ren/Camera.h
                eng/ren/Camera.cpp
				eng/ren/Material.h
				eng/ren/Material.cpp
				eng/ren/Matrices_old.h
				eng/ren/Matrices_old.cpp
				eng/ren/Mesh.h
				eng/ren/Mesh.cpp
				eng/ren/Plane.h
				eng/ren/Plane.cpp
				eng/ren/Program.h
				eng/ren/Program.cpp
				eng/ren/RenderState.h
				eng/ren/RenderStateSW.h
				eng/ren/RenderState.cpp
				eng/ren/RenderThread.h
				eng/ren/RenderThread.cpp
				eng/ren/RingBuffer.h
				eng/ren/SparseArray.h
				eng/ren/Texture.h
				eng/ren/Texture.cpp
				eng/ren/TextureSW.cpp)
                 
list(APPEND ALL_SOURCE_FILES ${REN_SOURCES})
set_source_files_properties(${REN_SOURCES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\eng\\ren" FILES ${REN_SOURCES})

list(APPEND ALL_SOURCE_FILES eng/ren/_ren.cpp)
source_group("src\\eng\\ren" FILES eng/ren/_ren.cpp)

set(SYS_FILES eng/sys/AssetFile.h
              eng/sys/AssetFile.cpp
              eng/sys/AssetFileIO.h
              eng/sys/AssetFileIO.cpp
              eng/sys/Delegate.h
              eng/sys/Json.h
              eng/sys/Json.cpp
              eng/sys/Log.h
              eng/sys/Optional.h
              eng/sys/Pack.h
              eng/sys/Pack.cpp
              eng/sys/Signal_.h
              eng/sys/SpinLock.h
              eng/sys/ThreadPool.h
              eng/sys/ThreadWorker.h
              eng/sys/Time_.h
              eng/sys/Time.cpp)
              
list(APPEND ALL_SOURCE_FILES ${SYS_FILES})
set_source_files_properties(${SYS_FILES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\eng\\sys" FILES ${SYS_FILES})

list(APPEND ALL_SOURCE_FILES eng/sys/_sys.cpp)
source_group("src\\eng\\sys" FILES eng/sys/_sys.cpp)

set(UI_FILES eng/ui/BaseElement.h
             eng/ui/BaseElement.cpp
             eng/ui/BitmapFont.h
             eng/ui/BitmapFont.cpp
             eng/ui/ButtonBase.h
             eng/ui/ButtonBase.cpp
             eng/ui/ButtonImage.h
             eng/ui/ButtonImage.cpp
             eng/ui/ButtonText.h
             eng/ui/ButtonText.cpp
             eng/ui/Cursor.h
             eng/ui/Cursor.cpp
             eng/ui/EditBox.h
             eng/ui/EditBox.cpp
             eng/ui/Frame.h
             eng/ui/Frame.cpp
             eng/ui/Image.h
             eng/ui/Image.cpp
             eng/ui/LinearLayout.h
             eng/ui/LinearLayout.cpp
             eng/ui/Renderer.h
			 eng/ui/RendererSW.cpp
             eng/ui/TypeMesh.h
             eng/ui/TypeMesh.cpp
             eng/ui/Utils.h
             eng/ui/Utils.cpp)
             
list(APPEND ALL_SOURCE_FILES ${UI_FILES})
set_source_files_properties(${UI_FILES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\eng\\ui" FILES ${UI_FILES})

list(APPEND ALL_SOURCE_FILES eng/ui/_ui.cpp)
source_group("src\\eng\\ui" FILES eng/ui/_ui.cpp)

set(GUI_SOURCES gui/CodeLockUI.h
				gui/CodeLockUI.cpp
				gui/DrawTestUI.h
				gui/DrawTestUI.cpp
				gui/FontStorage.h
				gui/GBackground.h
				gui/GBackground.cpp
				gui/GCursor.h
				gui/GCursor.cpp
				gui/LoadingIcon.h
				gui/LoadingIcon.cpp
                gui/LoadingIconSW.cpp
				gui/LoadingUI.h
				gui/LoadingUI.cpp
				gui/MenuUI.h
				gui/MenuUI.cpp)

list(APPEND ALL_SOURCE_FILES ${GUI_SOURCES})
set_source_files_properties(${GUI_SOURCES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\gui" FILES ${GUI_SOURCES})

list(APPEND ALL_SOURCE_FILES gui/_gui.cpp)
source_group("src\\gui" FILES gui/_gui.cpp)

set(MANAGERS_SOURCES managers/PlayerController.h
					 managers/PlayerController.cpp
					 managers/Renderer.h
					 managers/Renderer.cpp
					 managers/RendererSW.cpp
                     managers/SceneManager.h
                     managers/SceneManager.cpp)

list(APPEND ALL_SOURCE_FILES ${MANAGERS_SOURCES})
set_source_files_properties(${MANAGERS_SOURCES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\managers" FILES ${MANAGERS_SOURCES})

list(APPEND ALL_SOURCE_FILES managers/_managers.cpp)
source_group("src\\managers" FILES managers/_managers.cpp)

set(STATES_SOURCES states/GSCreate.h
                   states/GSCreate.cpp
				   states/GSDrawTest.h
				   states/GSDrawTest.cpp
				   states/GSDrawTestSW.cpp
                   states/GSMenu.h
				   states/GSMenu.cpp
				   states/GSLoading.h
				   states/GSLoading.cpp
				   states/GSRunning.h
				   states/GSRunning.cpp
				   states/GSTransition.h
				   states/GSTransition.cpp)

list(APPEND ALL_SOURCE_FILES ${STATES_SOURCES})
set_source_files_properties(${STATES_SOURCES} PROPERTIES HEADER_FILE_ONLY 1)
source_group("src\\states" FILES ${STATES_SOURCES})

list(APPEND ALL_SOURCE_FILES states/_states.cpp)
source_group("src\\states" FILES states/_states.cpp)

add_executable(the_game ${ALL_SOURCE_FILES})
target_link_libraries(the_game SW ${LIBS})

cmake_minimum_required(VERSION 2.8.4)
project(sys)

IF(WIN32)

ELSE(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(LIBS pthread)
ENDIF(WIN32)

set(SOURCE_FILES AssetFile.h
                 AssetFile.cpp
                 AssetFileIO.h
                 AssetFileIO.cpp
                 Delegate.h
                 Json.h
                 Json.cpp
                 Log.h
                 Optional.h
                 Pack.h
                 Pack.cpp
                 Signal_.h
                 SpinLock.h
                 ThreadPool.h
                 ThreadWorker.h
                 Time_.h
                 Time.cpp)

list(APPEND ALL_SOURCE_FILES ${SOURCE_FILES})
set_source_files_properties(${SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("src" FILES ${SOURCE_FILES})

list(APPEND ALL_SOURCE_FILES _sys.cpp)
source_group("src" FILES _sys.cpp)

add_library(sys STATIC ${ALL_SOURCE_FILES})
target_link_libraries(sys ${LIBS})

add_subdirectory(tests)
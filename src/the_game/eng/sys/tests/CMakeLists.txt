cmake_minimum_required(VERSION 3.2)
project(test_sys)

add_executable(test_sys main.cpp
                        test_common.h
                        test_json.cpp
                        test_optional.cpp
                        test_pack.cpp
                        test_signal.cpp)

target_link_libraries(test_sys sys)
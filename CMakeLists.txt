cmake_minimum_required(VERSION 2.8)
project(sylar)
set(CMAKE_VERBOSE_MAKEFILE ON)   #输出具体的信息
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -O3 -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")
set(LIB/SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    )

include_directories(.)
include_directories(~/yaml-cpp/include)

link_directories(/home/sz123/yaml-cpp)

add_library(sylar SHARED ${LIB/SRC})

add_executable(test tests/test.cc)
add_dependencies(test sylar)
target_link_libraries(test sylar yaml-cpp)       #这里sylar需要yaml-cpp

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar yaml-cpp)
target_link_libraries(test_config sylar yaml-cpp)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


cmake_minimum_required(VERSION 2.8)
project(sylar)
include(cmake/utils.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)   #输出具体的信息
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -O0 -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function  -Wno-builtin-macro-redefined")
set(LIB/SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    sylar/thread.cc
    sylar/fiber.cc
    sylar/scheduler.cc
    )

include_directories(.)

include_directories(~/yaml-cpp/include)
link_directories(/home/sz123/yaml-cpp)

add_library(sylar SHARED ${LIB/SRC})

set(LIBS
        sylar
        pthread
        yaml-cpp
        )         #这里sylar需要yaml-cpp

add_executable(test tests/test.cc)
add_dependencies(test sylar)       #作用是：编译test时会检查sylar是否已经产生了
force_redefine_file_macro_for_sources(test)     # 文件完整路径
target_link_libraries(test ${LIBS})       #前面依赖后面的

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar )         
force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_config ${LIBS})


add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sylar )         
force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${LIBS})

add_executable(test_util tests/test_util.cc)
add_dependencies(test_util sylar )         
force_redefine_file_macro_for_sources(test_util)
target_link_libraries(test_util ${LIBS})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber sylar )         
force_redefine_file_macro_for_sources(test_fiber)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler sylar )         
force_redefine_file_macro_for_sources(test_scheduler)
target_link_libraries(test_scheduler ${LIBS})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


cmake_minimum_required(VERSION 2.8)
project(sylar)
include(cmake/utils.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)   #输出具体的信息
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -O0 -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function  -Wno-builtin-macro-redefined")
set(LIB/SRC
    sylar/address.cc
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    sylar/thread.cc
    sylar/fiber.cc
    sylar/scheduler.cc
    sylar/iomanager.cc
    sylar/timer.cc
    sylar/hook.cc
    sylar/fd_manager.cc
    )

include_directories(.)

include_directories(~/yaml-cpp/include)
link_directories(/home/sz123/yaml-cpp)

add_library(sylar SHARED ${LIB/SRC})

set(LIBS
        sylar
        pthread
        yaml-cpp
        dl
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

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager sylar )         
force_redefine_file_macro_for_sources(test_iomanager)
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook sylar )         
force_redefine_file_macro_for_sources(test_hook)
target_link_libraries(test_hook ${LIBS})

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address sylar )         
force_redefine_file_macro_for_sources(test_address)
target_link_libraries(test_address ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


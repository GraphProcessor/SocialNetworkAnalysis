cmake_minimum_required(VERSION 3.3)
project(Codes_Yche)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14  -g")

#Add Boost Includes
find_package(Boost 1.58.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
#target_link_libraries(XXX ${Boost_LIBRARIES})

set(SOURCE_FILES test_daemon.cpp damon_algorithm.cpp damon_algorithm.h include_header.h)
add_executable(Daemon_Test ${SOURCE_FILES})
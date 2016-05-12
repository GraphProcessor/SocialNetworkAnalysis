cmake_minimum_required(VERSION 3.3)
project(Codes_Yche)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14  -O3")

#Add Boost Includes
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Threads)

add_executable(Daemon_Test test_daemon.cpp damon_algorithm.cpp damon_algorithm.h include_header.h parallelizer.h reducer.h input_output_handler.h)
target_link_libraries(Daemon_Test ${CMAKE_THREAD_LIBS_INIT})

add_executable(CISTest test_cis.cpp cis_algorithm.cpp cis_algorithm.h include_header.h parallelizer.h reducer.h input_output_handler.h)
target_link_libraries(CISTest ${CMAKE_THREAD_LIBS_INIT})

add_executable(GceTest test_gce.cpp gce_algorithm.cpp gce_algorithm.h include_header.h)


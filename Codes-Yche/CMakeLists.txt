cmake_minimum_required(VERSION 2.8)
project(Codes_Yche)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14  -O3")

#set(CMAKE_CXX_COMPILER "/ghome/yche/usr/local/bin/g++")

find_package(Boost REQUIRED COMPONENTS regex)
message(${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIRS})
find_package(Threads)
message(${Boost_INCLUDE_DIRS})
message(${CMAKE_THREAD_LIBS_INIT})

set(ParallelUtilFiles include_header.h parallelizer.h reducer.h)
set(GraphIOUtilFiles input_output_handler.h)
set(DemonFiles test_daemon.cpp damon_algorithm.cpp damon_algorithm.h)
set(CISFiles test_cis.cpp cis_algorithm.cpp cis_algorithm.h)
set(GCEFiles test_gce.cpp gce_algorithm.cpp gce_algorithm.h)

add_executable(Daemon_Test  ${ParallelUtilFiles} ${GraphIOUtilFiles} ${DemonFiles})
target_link_libraries(Daemon_Test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(Daemon_Test ${Boost_LIBRARIES})

add_executable(CISTest ${ParallelUtilFiles} ${GraphIOUtilFiles} ${CISFiles})
target_link_libraries(CISTest ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(CISTest ${Boost_LIBRARIES})

#add_executable(GceTest ${GCEFiles} include_header.h)

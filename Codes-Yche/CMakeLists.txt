cmake_minimum_required(VERSION 3.3)
project(Codes_Yche)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14  -g")

#Add Boost Includes
find_package(Boost 1.58.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
#target_link_libraries(XXX ${Boost_LIBRARIES})

add_executable(Daemon_Test test_daemon.cpp damon_algorithm.cpp damon_algorithm.h include_header.h)
add_executable(CISTest test_cis.cpp cis_algorithm.cpp cis_algorithm.h include_header.h)
add_executable(GceTest test_gce.cpp gce_algorithm.cpp gce_algorithm.h include_header.h)
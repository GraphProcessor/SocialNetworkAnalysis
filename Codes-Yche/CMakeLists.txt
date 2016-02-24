cmake_minimum_required(VERSION 3.3)
project(Codes_Yche)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#Add Boost Includes
find_package(Boost 1.58.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
#target_link_libraries(XXX ${Boost_LIBRARIES})

set(SOURCE_FILES main.cpp damon_algorithm.cpp damon_algorithm.h)
add_executable(Codes_Yche ${SOURCE_FILES})
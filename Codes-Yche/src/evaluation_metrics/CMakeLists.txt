cmake_minimum_required(VERSION 3.3)
project(OCD_Evaluation_Yche)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3")

#Add Boost Includes
find_package(Boost COMPONENTS system filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES modularity.cpp modularity.h include_header.h input_output_handler.h)
add_executable(OCD_Evaluation_Yche ${SOURCE_FILES})
target_link_libraries(OCD_Evaluation_Yche ${Boost_LIBRARIES})

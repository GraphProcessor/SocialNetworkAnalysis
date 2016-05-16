cmake_minimum_required(VERSION 3.5)
project(OCD_Evaluation_Yche)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

#Add Boost Includes
find_package(Boost COMPONENTS REGEX REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp modularity.cpp modularity.h include_header.h input_output_handler.h)
add_executable(OCD_Evaluation_Yche ${SOURCE_FILES})
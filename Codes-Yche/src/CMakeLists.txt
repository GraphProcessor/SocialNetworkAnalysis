cmake_minimum_required(VERSION 3.4)
project(Codes_Yche)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -g")


find_package(Boost REQUIRED COMPONENTS regex thread)
message(${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIRS})

find_package(Threads)
message(${Boost_INCLUDE_DIRS})
message(${CMAKE_THREAD_LIBS_INIT})

include_directories(".")

set(ParallelUtilFiles parallel_utils/dataflow_scheduler.h parallel_utils/reduce_scheduler.h
        parallel_utils/fine_grained_merge_scheduler.h parallel_utils/parallel_configuration.h
        parallel_utils/thread_pool_breakable.h parallel_utils/thread_pool_base.h)
set(GraphIOUtilFiles input_output_handler.h)
set(DemonFiles test_demon.cpp algorithm/demon_algorithm.cpp algorithm/demon_algorithm.h)
set(CISFiles test_cis.cpp algorithm/cis_algorithm.cpp algorithm/cis_algorithm.h)

add_executable(DemonAlgo ${ParallelUtilFiles} ${GraphIOUtilFiles} ${DemonFiles})
target_link_libraries(DemonAlgo ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(DemonAlgo ${Boost_LIBRARIES})

add_executable(CISAlgo ${ParallelUtilFiles} ${GraphIOUtilFiles} ${CISFiles})
target_link_libraries(CISAlgo ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(CISAlgo ${Boost_LIBRARIES})

add_subdirectory(test)
add_subdirectory(evaluation_metrics)
add_subdirectory(demo)

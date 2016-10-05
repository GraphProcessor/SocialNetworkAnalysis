add_executable(demo_cis demo_cis.cpp ../algorithm/cis_algorithm.cpp)
target_link_libraries(demo_cis ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(demo_cis ${Boost_LIBRARIES})
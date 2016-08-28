add_executable(ThreadPoolTest ../parallel_utils/thread_pool_base.h ./thread_pool_test.cpp)
target_link_libraries(ThreadPoolTest ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

add_executable(ThreadPoolBreakableTest ../parallel_utils/thread_pool_base.h ../parallel_utils/thread_pool_breakable.h ./thread_pool_breakable_test.cpp)
target_link_libraries(ThreadPoolBreakableTest ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
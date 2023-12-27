find_package(GTest REQUIRED)

include_directories(
    ../include
    ${GTEST_INCLUDE_DIRS}
    )
add_executable(object_pool_test object_pool_test.cpp)
target_link_libraries(object_pool_test ${GTEST_BOTH_LIBRARIES} pthread)
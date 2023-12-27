cmake_minimum_required(VERSION 3.8)
project(object_pool)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

add_executable(demo src/demo.cpp)
target_link_libraries(demo pthread)
target_include_directories(demo PUBLIC include)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()


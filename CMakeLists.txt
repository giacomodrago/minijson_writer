# You don't need to compile any library to use minijson_writer in your project:
# just include the minijson_writer.hpp header anywhere you need, and you're ready to go.

# Use this cmake project to build and run the unit tests (you will need Google Test):
#   $ mkdir build
#   $ cd build
#   $ cmake ..
#   $ make
#   $ ./minijson_writer_tests
# You can append -DCPP11:int=1 to the cmake command to build in C++11 mode.

cmake_minimum_required(VERSION 2.8.7)
project(minijson_writer)

if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unknown-pragmas")

    if(CPP11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")
        find_package(Boost 1.55 REQUIRED)
        include_directories(${Boost_INCLUDE_DIRS})
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef")
endif()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

enable_testing()

add_executable(minijson_writer_tests
    minijson_writer_tests.cpp
)

target_link_libraries(minijson_writer_tests
    ${GTEST_BOTH_LIBRARIES})

if(UNIX)
    target_link_libraries(minijson_writer_tests
        pthread)
endif()

add_test(NAME all
    COMMAND minijson_writer_tests)

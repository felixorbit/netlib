cmake_minimum_required(VERSION 3.5)

project(cppnetlib LANGUAGES CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()


# choose whether to build test examples
if(CMAKE_PROJECT_NAME STREQUAL "cppnetlib")
    option(CPPNETLIB_BUILD_TESTS "Build test examples" ON)
endif()


# check c++ standard
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


# other compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wshadow -Wpointer-arith")
set(CMAKE_CXX_FLAGS_DEBUG "-O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


# third-party libs
find_package(fmt 6.2.0 REQUIRED)
if(fmt_FOUND)
    message("fmt found")
endif()

include_directories(${PROJECT_SOURCE_DIR})


# show compile flags
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})


# add subdirectories
add_subdirectory(catch2)
add_subdirectory(cppnetlib/core)
add_subdirectory(cppnetlib/base)
add_subdirectory(cppnetlib/net)
add_subdirectory(cppnetlib/http)

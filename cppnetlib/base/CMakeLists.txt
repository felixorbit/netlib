project(cppnetlib_base)

include(CheckFunctionExists)
check_function_exists(accept4 HAVE_ACCEPT4)

if(NOT HAVE_ACCEPT4)
    message("accept4 not found")
    set_source_files_properties(sock_utils.cc PROPERTIES COMPILE_FLAGS "-DNO_ACCEPT4")
endif()

if(CPPNETLIB_BUILD_TESTS)
  add_subdirectory(test)
endif()

# generate link library
aux_source_directory(. DIR_BASE)
add_library(${PROJECT_NAME} ${DIR_BASE})
# add other link libraries
target_link_libraries(${PROJECT_NAME} cppnetlib_core)

project(cppnetlib_net)

if(CPPNETLIB_BUILD_TESTS)
  add_subdirectory(test)
endif()

aux_source_directory(. DIR_NET)
add_library(${PROJECT_NAME} ${DIR_NET})
target_link_libraries(${PROJECT_NAME} cppnetlib_core cppnetlib_base)
project(cppnetlib_http)

if(CPPNETLIB_BUILD_TESTS)
  add_subdirectory(test)
endif()

aux_source_directory(. DIR_HTTP)
add_library(${PROJECT_NAME} ${DIR_HTTP})
target_link_libraries(${PROJECT_NAME} cppnetlib_net cppnetlib_core cppnetlib_base)
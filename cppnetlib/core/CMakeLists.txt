project(cppnetlib_core)

if(CPPNETLIB_BUILD_TESTS)
  add_subdirectory(test)
endif()

aux_source_directory(. DIR_CORE)
add_library(${PROJECT_NAME} ${DIR_CORE})
target_link_libraries(${PROJECT_NAME} pthread rt fmt::fmt)

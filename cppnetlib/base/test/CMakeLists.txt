add_executable(test_InetAddr test_inet_addr.cc)
target_link_libraries(test_InetAddr cppnetlib_base Catch2::Test)
add_test(NAME test_InetAddr COMMAND test_InetAddr)

add_executable(test_Buffer test_buffer.cc)
target_link_libraries(test_Buffer cppnetlib_base Catch2::Test)
add_test(NAME test_Buffer COMMAND test_Buffer)
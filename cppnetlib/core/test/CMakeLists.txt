add_executable(test_Thread test_thread.cc)
target_link_libraries(test_Thread cppnetlib_core)

add_executable(test_TimeAnchor test_time_anchor.cc)
target_link_libraries(test_TimeAnchor cppnetlib_core)

add_executable(test_Mutex test_mutex.cc)
target_link_libraries(test_Mutex cppnetlib_core)

add_executable(test_fmt test_fmt.cc)
target_link_libraries(test_fmt fmt::fmt)

add_executable(test_LogStream test_log_stream.cc)
target_link_libraries(test_LogStream cppnetlib_core Catch2::Test)
add_test(NAME test_LogStream COMMAND test_LogStream)

add_executable(bench_LogStream bench_log_stream.cc)
target_link_libraries(bench_LogStream cppnetlib_core)

add_executable(test_ThreadPool test_thread_pool.cc)
target_link_libraries(test_ThreadPool cppnetlib_core)

add_executable(test_LogFile test_log_file.cc)
target_link_libraries(test_LogFile cppnetlib_core)

add_executable(test_AsyncLog test_async_log.cc)
target_link_libraries(test_AsyncLog cppnetlib_core)
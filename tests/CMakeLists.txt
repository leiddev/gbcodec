add_executable(smoke_test smoke_test.c)
target_link_libraries(smoke_test PRIVATE gbcodec)
gbcodec_enable_warnings(smoke_test)

add_test(NAME smoke_test COMMAND smoke_test)

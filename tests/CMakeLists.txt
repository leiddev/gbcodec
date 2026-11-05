add_executable(test_utf8 test_utf8.c)
target_link_libraries(test_utf8 PRIVATE gbcodec)
gbcodec_enable_warnings(test_utf8)
add_test(NAME test_utf8 COMMAND test_utf8)

add_executable(test_utf16 test_utf16.c)
target_link_libraries(test_utf16 PRIVATE gbcodec)
gbcodec_enable_warnings(test_utf16)
add_test(NAME test_utf16 COMMAND test_utf16)

add_executable(test_gb2312 test_gb2312.c)
target_link_libraries(test_gb2312 PRIVATE gbcodec)
gbcodec_enable_warnings(test_gb2312)
add_test(NAME test_gb2312 COMMAND test_gb2312)

add_executable(test_string test_string.c)
target_link_libraries(test_string PRIVATE gbcodec)
gbcodec_enable_warnings(test_string)
add_test(NAME test_string COMMAND test_string)

add_executable(test_roundtrip test_roundtrip.c)
target_link_libraries(test_roundtrip PRIVATE gbcodec)
gbcodec_enable_warnings(test_roundtrip)
add_test(NAME test_roundtrip COMMAND test_roundtrip)

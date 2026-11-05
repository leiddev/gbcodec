# Unit tests. Each test program is built as a separate executable and
# registered with CTest; there is no third-party test framework.

set(GBCODEC_C_TESTS
    test_convert
    test_gb2312
    test_replace
    test_roundtrip
    test_utf8
    test_utf16)

foreach(test_name IN LISTS GBCODEC_C_TESTS)
    add_executable(${test_name} ${test_name}.c)
    target_link_libraries(${test_name} PRIVATE gbcodec)
    gbcodec_enable_warnings(${test_name})
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

if(GBCODEC_BUILD_CXX)
    add_executable(test_cpp test_cpp.cpp)
    target_link_libraries(test_cpp PRIVATE gbcodec_cpp)
    gbcodec_enable_warnings(test_cpp)
    add_test(NAME test_cpp COMMAND test_cpp)

    # The wrapper must also work without std::string_view; with C++11 it uses
    # its own fallback view, which this target exercises.
    add_executable(test_cpp11 test_cpp.cpp)
    target_link_libraries(test_cpp11 PRIVATE gbcodec_cpp)
    set_target_properties(test_cpp11 PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)
    gbcodec_enable_warnings(test_cpp11)
    add_test(NAME test_cpp11 COMMAND test_cpp11)
endif()

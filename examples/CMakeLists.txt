# Example programs. They are built by default only when gbcodec is the
# top-level project.

add_executable(unicode_example unicode_example.c)
target_link_libraries(unicode_example PRIVATE gbcodec)
gbcodec_enable_warnings(unicode_example)

if(GBCODEC_BUILD_CXX)
    add_executable(unicode_cpp_example unicode_cpp_example.cpp)
    target_link_libraries(unicode_cpp_example PRIVATE gbcodec_cpp)
    gbcodec_enable_warnings(unicode_cpp_example)
endif()

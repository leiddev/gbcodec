# Example programs. They are built by default only when gbcodec is the
# top-level project.

add_executable(gbcodec_example gbcodec_example.c)
target_link_libraries(gbcodec_example PRIVATE gbcodec)
gbcodec_enable_warnings(gbcodec_example)

if(GBCODEC_BUILD_CXX)
    add_executable(gbcodec_cpp_example gbcodec_cpp_example.cpp)
    target_link_libraries(gbcodec_cpp_example PRIVATE gbcodec_cpp)
    gbcodec_enable_warnings(gbcodec_cpp_example)
endif()

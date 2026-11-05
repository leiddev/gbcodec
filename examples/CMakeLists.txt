add_executable(gbcodec-example-c unicode_example.c)
target_link_libraries(gbcodec-example-c PRIVATE gbcodec)
gbcodec_enable_warnings(gbcodec-example-c)

add_executable(gbcodec-example-cpp unicode_cpp_example.cpp)
target_link_libraries(gbcodec-example-cpp PRIVATE gbcodec_cpp)
gbcodec_enable_warnings(gbcodec-example-cpp)

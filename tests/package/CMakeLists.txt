cmake_minimum_required(VERSION 3.15)

# Consumer project used to smoke-test the installed package: it must find
# gbcodec through find_package(gbcodec CONFIG) and link the exported targets.
project(gbcodec_consumer LANGUAGES C)

find_package(gbcodec CONFIG REQUIRED)

add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE gbcodec::gbcodec)

if(gbcodec_CXX_AVAILABLE)
    enable_language(CXX)
    add_executable(consumer_cpp main.cpp)
    target_link_libraries(consumer_cpp PRIVATE gbcodec::gbcodec_cpp)
endif()

enable_testing()
add_test(NAME consumer COMMAND consumer)
if(gbcodec_CXX_AVAILABLE)
    add_test(NAME consumer_cpp COMMAND consumer_cpp)
endif()

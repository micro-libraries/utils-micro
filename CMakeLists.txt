cmake_minimum_required(VERSION 3.30)
project(utils-micro VERSION 0.4.0 LANGUAGES C CXX)

include("cmake/micro-libraries.cmake")

# Create target "utils-micro-dev".
add_header_only_micro_library("${PROJECT_NAME}-dev" SOURCE_DIR "${CMAKE_SOURCE_DIR}/include/dev")

add_subdirectory(src)

add_subdirectory(test)

# utils-micro-dev is installed through pure copy in `conan/conanfile-dev.py`
install(TARGETS utils-micro)

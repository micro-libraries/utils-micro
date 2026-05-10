cmake_minimum_required(VERSION 3.30)
project(utils-micro VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE UTILS_MICRO_DEV_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/dev/*")

add_library(utils-micro-dev INTERFACE)
target_sources(utils-micro-dev INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include/dev
    FILES ${UTILS_MICRO_DEV_HEADERS}
)
target_include_directories(utils-micro-dev INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/dev>
    $<INSTALL_INTERFACE:include>
)
add_library(utils-micro::utils-micro-dev ALIAS utils-micro-dev)

cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)

include("cmake/simd-flags.cmake")
include("cmake/release-debug-flags.cmake")

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

install(TARGETS utils-micro)

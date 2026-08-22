cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT INTERPROCEDURAL_OPTIMIZATION OUTPUT IPO_OUTPUT)
if(NOT ${INTERPROCEDURAL_OPTIMIZATION})
    message(STATUS "IPO not supported: ${IPO_OUTPUT}")
endif()

include("${CMAKE_SOURCE_DIR}/cmake/simd-flags.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/release-debug-flags.cmake")

# Create target "utils-micro".
add_binary_wrapper_micro_library("${PROJECT_NAME}"
        CONFIGURATION default
        HEADER "${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}.h"
        SOURCES "${PROJECT_NAME}.cpp"
        DEPENDENCIES "${PROJECT_NAME}-dev"
)
set_property(TARGET utils-micro PROPERTY INTERPROCEDURAL_OPTIMIZATION ${INTERPROCEDURAL_OPTIMIZATION})

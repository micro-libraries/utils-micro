enable_testing()

find_package(Threads REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(OpenMP)

include(Catch)

include("${CMAKE_SOURCE_DIR}/cmake/tests-with-purpose.cmake")

add_unit_test(test-matrix-view TESTED_LIBRARY "${PROJECT_NAME}-dev")

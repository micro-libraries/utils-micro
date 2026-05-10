find_package(Threads REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(OpenMP)

include(Catch)

function(add_unit_test TARGET_NAME)
    cmake_parse_arguments(X_TEST "" "SOURCE" "" ${ARGN})
    add_executable(${TARGET_NAME} ${X_TEST_SOURCE})
    target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(${TARGET_NAME} PRIVATE utils-micro::utils-micro-dev Catch2::Catch2WithMain)
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
    target_compile_definitions(${TARGET_NAME} PRIVATE ${X_CONFIGURATION})
    catch_discover_tests(${TARGET_NAME} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

add_unit_test(test-matrix-view SOURCE test-matrix-view.cpp)

function(add_header_only_micro_library TARGET_NAME)
    message(${ARGN})
    cmake_parse_arguments(LIB_ARG "" "SOURCE_DIR" "DEPENDENCIES" ${ARGN})

    file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS "${LIB_ARG_SOURCE_DIR}/*")

    add_library("${TARGET_NAME}" INTERFACE)
    target_sources("${TARGET_NAME}" INTERFACE FILE_SET HEADERS FILES ${LIB_HEADERS})
    target_include_directories("${TARGET_NAME}" INTERFACE
            $<BUILD_INTERFACE:${LIB_ARG_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries("${TARGET_NAME}" INTERFACE ${LIB_ARG_DEPENDENCIES})

    add_library("${PROJECT_NAME}::${TARGET_NAME}" ALIAS "${TARGET_NAME}")
endfunction()

# Defines a binary wrapper (static or shared) library for a development (header-only) library.
# This is usually intended for testing or whenever a standalone library is considered more suitable for use
# in a downstream project than directly including development headers and writing a custom configuration.

# A wrapper uses a single public header (preferably named like the target) and a single configuration header
# whose name is specified by the target name and Conan option `configuration`, along with source files.
# Both the public header and the configuration header should be placed in the `include/` folder.
function(add_binary_wrapper_micro_library TARGET_NAME)
    cmake_parse_arguments(LIB_ARG "" "CONFIGURATION;HEADER" "SOURCES;DEPENDENCIES" ${ARGN})

    set(CONFIGURATION_FILENAME "${TARGET_NAME}.config.${LIB_ARG_CONFIGURATION}.h")

    add_library("${TARGET_NAME}")
    target_sources("${TARGET_NAME}" PRIVATE ${LIB_ARG_SOURCES})
    target_sources("${TARGET_NAME}" PUBLIC
            FILE_SET HEADERS
            BASE_DIRS "${CMAKE_SOURCE_DIR}/include"
            FILES ${LIB_ARG_HEADER}
    )
    target_compile_definitions("${TARGET_NAME}" PRIVATE CONFIGURATION "${CONFIGURATION_FILENAME}")
    target_include_directories("${TARGET_NAME}" PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries("${TARGET_NAME}" PRIVATE ${LIB_ARG_DEPENDENCIES})

    add_library("${PROJECT_NAME}::${TARGET_NAME}" ALIAS "${TARGET_NAME}")
endfunction()

# Fairly generic function for adding a binary library (non-interface) with sources and headers.
# Intended for fully fleshed out libraries without a development component.
# It also expects a configuration header and any number of public or private headers
# in a given include folder (typically `include/`).
function(add_micro_library TARGET_NAME)
    cmake_parse_arguments(LIB_ARG "" "CONFIGURATION;INCLUDE_DIR" "SOURCES;PUBLIC_HEADERS;DEPENDENCIES" ${ARGN})

    set(CONFIGURATION_FILENAME "${TARGET_NAME}.config.${LIB_ARG_CONFIGURATION}.h")

    add_library("${TARGET_NAME}")
    target_sources("${TARGET_NAME}" PRIVATE ${LIB_ARG_SOURCES})
    target_sources("${TARGET_NAME}" PUBLIC
            FILE_SET HEADERS
            BASE_DIRS "${LIB_ARG_INCLUDE_DIR}"
            FILES ${LIB_ARG_PUBLIC_HEADERS}
    )
    target_compile_definitions("${TARGET_NAME}" PRIVATE CONFIGURATION "${CONFIGURATION_FILENAME}")
    target_include_directories("${TARGET_NAME}" PUBLIC
            $<BUILD_INTERFACE:${LIB_ARG_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries("${TARGET_NAME}" PRIVATE ${LIB_ARG_DEPENDENCIES})

    add_library("${PROJECT_NAME}::${TARGET_NAME}" ALIAS "${TARGET_NAME}")
endfunction()


add_library(utils-micro SHARED utils-micro.cpp)
target_sources(utils-micro PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ../include
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/utils-micro.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/utils-micro-config.h
)
target_include_directories(utils-micro PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/dev>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(utils-micro PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(utils-micro PRIVATE utils-micro-dev)
add_library(utils-micro::utils-micro ALIAS utils-micro)

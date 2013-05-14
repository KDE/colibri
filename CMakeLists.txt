project(colibri)
cmake_minimum_required(VERSION 2.6)
set(COLIBRI_VERSION "0.3.0")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(KDE4 4.4 REQUIRED)
include (KDE4Defaults)

configure_file(buildconfig.h.in ${CMAKE_BINARY_DIR}/buildconfig.h @ONLY)

include_directories(
    ${KDE4_INCLUDES}
    ${QT_INCLUDES}
    ${CMAKE_BINARY_DIR}
    )

add_subdirectory(app)
add_subdirectory(kcm)

project(colibri)
cmake_minimum_required(VERSION 2.6)
set(PROJECT_VERSION "0.1.1")

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
    | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(KDE4 4.4 REQUIRED)
include (KDE4Defaults)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})

add_subdirectory(app)
add_subdirectory(kcm)
add_subdirectory(po)

project(colibri)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})

add_subdirectory(app)
add_subdirectory(kcm)

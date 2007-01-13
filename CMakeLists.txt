project(kthinkbat_panelapplet)
cmake_minimum_required(VERSION 2.4.2)

find_package(Qt3 REQUIRED)
find_package(KDE3 REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})

link_directories(${KDE3_LIB_DIR})

include_directories(${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR})

add_subdirectory(src)

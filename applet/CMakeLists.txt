project(kthinkbat-plasma-applet)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PLASMA_INCLUDE_DIR})

set(kthinkbat-applet_SRCS 
    kthinkbat.cpp)

kde4_add_ui_files(kthinkbat-applet_SRCS prefs.ui)
kde4_add_plugin(plasma_applet_kthinkbat ${kthinkbat-applet_SRCS})
target_link_libraries(plasma_applet_kthinkbat plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_kthinkbat DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kthinkbat.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kthinkbat.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

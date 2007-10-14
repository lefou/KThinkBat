project(kthinkbat-plasma-dataengine)

set(kthinkbat-dataengine_SRCS 
    batinfo.cpp)

kde4_add_plugin(plasma_engine_kthinkbat ${kthinkbat-dataengine_SRCS})
target_link_libraries(plasma_engine_kthinkbat ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} plasma)

install(TARGETS plasma_engine_kthinkbat DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-kthinkbat.desktop DESTINATION ${SERVICES_INSTALL_DIR})

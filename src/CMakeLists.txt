
set(kthinkbat_panelapplet_PART_SRCS 
    src/kthinkbat.cpp
    src/batgauge.cpp
    src/batinfo.cpp
    src/battooltip.cpp
    )

kde3_automoc(${kthinkbat_panelapplet_PART_SRCS})

#kde3_add_plugin(kthinkbat_panelapplet ${kthinkbat_panelapplet_PART_SRCS})

target_link_libraries(kthinkbat_panelapplet ${KDE3_KDEUI_LIBS})
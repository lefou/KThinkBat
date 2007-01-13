
set(kthinkbat_panelapplet_LIB_SRCS 
    kthinkbat.cpp
    batgauge.cpp
    batinfo.cpp
    battooltip.cpp
    )

kde3_automoc(${kthinkbat_panelapplet_LIB_SRCS})

kde3_add_kcfg_files(kthinkbat_panelapplet_LIB_SRCS kthinkbatconfig.kcfgc)

kde3_add_ui_files(kthinkbat_panelapplet_LIB_SRCS prefs.ui)

#add_library(kthinkbat_panelapplet SHARED ${kthinkbat_panelapplet_LIB_SRCS})

kde3_add_kpart(kthinkbat_panelapplet ${kthinkbat_panelapplet_LIB_SRCS})

target_link_libraries(kthinkbat_panelapplet ${KDE3thi_KDEUI_LIBS})
SUBDIRS = icons
INCLUDES= $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kthinkbat_panelapplet.la

kthinkbat_panelapplet_la_SOURCES = batgauge.cpp batinfo.cpp batinfobase.cpp \
	batinfosum.cpp battooltip.cpp kthinkbat.cpp kthinkbatconfig.kcfgc prefs.ui
kthinkbat_panelapplet_la_LDFLAGS = -avoid-version -module $(all_libraries)
kthinkbat_panelapplet_la_LIBADD = $(LIB_KDEUI) 

kthinkbat_DATA = kthinkbat.desktop
kthinkbatdir = $(kde_datadir)/kicker/applets

kde_kcfg_DATA = kthinkbat.kcfg 

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kthinkbat.pot
noinst_HEADERS = batgauge.h batinfo.h batinfobase.h batinfosum.h battooltip.h

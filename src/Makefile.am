INCLUDES= $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = kthinkbat_panelapplet.la

kthinkbat_panelapplet_la_SOURCES = kthinkbat.cpp batinfo.cpp batgauge.cpp \
	kthinkbatconfig.kcfgc prefs.ui
kthinkbat_panelapplet_la_LDFLAGS = -avoid-version -module $(all_libraries)
kthinkbat_panelapplet_la_LIBADD = $(LIB_KDEUI) 

kthinkbat_DATA = kthinkbat.desktop
kthinkbatdir = $(kde_datadir)/kicker/applets

kde_kcfg_DATA = kthinkbat.kcfg 

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kthinkbat.pot
noinst_HEADERS = batinfo.h batgauge.h

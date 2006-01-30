FIND_PACKAGE(KDE3 REQUIRED)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})

LINK_DIRECTORIES(${KDE3_LIB_DIR})

ADD_SUBDIRECTORY( cvsservice ) 
ADD_SUBDIRECTORY( svnservice ) 
ADD_SUBDIRECTORY( plugins ) 
ADD_SUBDIRECTORY( pics ) 

INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(cervisia_KDEINIT_SRCS
main.cpp
cervisiashell.cpp
)

KDE3_AUTOMOC(${cervisia_KDEINIT_SRCS})

KDE3_ADD_KLM( cervisia ${cervisia_KDEINIT_SRCS})

TARGET_LINK_LIBRARIES(kdeinit_cervisia  ${QT_AND_KDECORE_LIBS} kparts cvsservice cervisialib )

INSTALL_TARGETS(/lib kdeinit_cervisia )

TARGET_LINK_LIBRARIES( cervisia kdeinit_cervisia )
INSTALL_TARGETS(/bin cervisia )

########### next target ###############

SET(cervisiapart_PART_SRCS
updateview.cpp
protocolview.cpp
changelogdlg.cpp
historydlg.cpp
repositorydlg.cpp
checkoutdlg.cpp
mergedlg.cpp
cvsdir.cpp
cervisiapart.cpp
addrepositorydlg.cpp
watchersdlg.cpp
updateview_items.cpp
updateview_visitors.cpp
entry_status.cpp
cvsinitdlg.cpp
editwithmenu.cpp
)

KDE3_AUTOMOC(${cervisiapart_PART_SRCS})

KDE3_ADD_KPART(cervisiapart WITH_PREFIX ${cervisiapart_PART_SRCS})

TARGET_LINK_LIBRARIES(cervisiapart  ${QT_AND_KDECORE_LIBS} kparts cvsservice cervisialib )

INSTALL_TARGETS(/lib/kde3 cervisiapart )


########### next target ###############

SET(cervisialib_STAT_SRCS
annotatedlg.cpp
diffdlg.cpp
patchoptiondlg.cpp
logdlg.cpp
progressdlg.cpp
resolvedlg.cpp
resolvedlg_p.cpp
annotateview.cpp
diffview.cpp
loglist.cpp
logplainview.cpp
logtree.cpp
loginfo.cpp
misc.cpp
qttableview.cpp
tooltip.cpp
settingsdlg.cpp
pluginbase.cpp
entry.cpp
ignorefilterbase.cpp
addremovedlg.cpp
commitdlg.cpp
stringmatcher.cpp
logmessageedit.cpp
output_parser.cpp
log_parser.cpp
commandbase.cpp
watchdlg.cpp
annotate_parser.cpp
repositories.cpp
update_parser.cpp
pluginmanager.cpp
updatedlg.cpp
tagdlg.cpp
diff_parser.cpp
checkoutwidgetbase.cpp
)

KDE3_AUTOMOC(${cervisialib_STAT_SRCS})

SET( cervisialib_UI
settingsdlg_advanced.ui
)

KDE3_ADD_UI_FILES(cervisialib_STAT_SRCS ${cervisialib_UI} )

SET( cervisialib_KCFG_SRCS
cervisiasettings.kcfgc
)

KDE3_ADD_KCFG_FILES(cervisialib_STAT_SRCS ${cervisialib_KCFG_SRCS})

ADD_LIBRARY(cervisialib STATIC ${cervisialib_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /share/applications/kde FILES cervisia.desktop )
INSTALL_FILES( /share/config.kcfg FILES cervisiapart.kcfg )
INSTALL_FILES( /share/apps/cervisiapart FILES cervisiaui.rc )
INSTALL_FILES( /share/apps/cervisia FILES cervisiashellui.rc eventsrc )
INSTALL_FILES( /share/apps/kconf_update FILES cervisia.upd )

KDE3_INSTALL_ICONS( hicolor )

#KDE3_PLACEHOLDER()



#original Makefile.am contents follow:

#CERVISIA_VERSION = 2.2.89
#INCLUDES = -I./cvsservice -D_BSD_SOURCE $(all_includes)
#
#SUBDIRS = cvsservice svnservice . plugins pics
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = cervisia.la
#kde_module_LTLIBRARIES = libcervisiapart.la
#noinst_LTLIBRARIES = libcervisialib.la
#
#libcervisialib_la_SOURCES = annotatedlg.cpp diffdlg.cpp patchoptiondlg.cpp logdlg.cpp \
#		   progressdlg.cpp resolvedlg.cpp resolvedlg_p.cpp annotateview.cpp \
#		   diffview.cpp loglist.cpp logplainview.cpp logtree.cpp \
#		   loginfo.cpp misc.cpp qttableview.cpp tooltip.cpp cervisiasettings.kcfgc \
#		   settingsdlg.cpp settingsdlg_advanced.ui pluginbase.cpp entry.cpp ignorefilterbase.cpp \
#		   addremovedlg.cpp commitdlg.cpp stringmatcher.cpp logmessageedit.cpp \
#		   output_parser.cpp log_parser.cpp commandbase.cpp watchdlg.cpp annotate_parser.cpp \
#		   repositories.cpp update_parser.cpp pluginmanager.cpp updatedlg.cpp tagdlg.cpp diff_parser.cpp \
#		   checkoutwidgetbase.cpp
#libcervisialib_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h
#
#libcervisiapart_la_SOURCES = updateview.cpp protocolview.cpp \
#		   changelogdlg.cpp historydlg.cpp \
#		   repositorydlg.cpp checkoutdlg.cpp \
#		   mergedlg.cpp cvsdir.cpp cervisiapart.cpp \
#		   addrepositorydlg.cpp watchersdlg.cpp \
#		   updateview_items.cpp updateview_visitors.cpp \
#		   entry_status.cpp cvsinitdlg.cpp \
#		   editwithmenu.cpp
#libcervisiapart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#libcervisiapart_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KUTILS) \
#		   cvsservice/libcvsservice.la libcervisialib.la
#libcervisiapart_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h
#
#cervisia_la_SOURCES = main.cpp cervisiashell.cpp
#cervisia_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
#cervisia_la_LIBADD = $(LIB_KPARTS) $(LIB_KUTILS) cvsservice/libcvsservice.la libcervisialib.la
#cervisia_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h
#
#man_MANS = cervisia.1
#
#METASOURCES = AUTO
#KDE_ICON = AUTO
#
#EXTRA_DIST = cervisia.desktop cervisia.png cervisia-small.png
#CLEANFILES = cervisia.1
#
#xdg_apps_DATA = cervisia.desktop
#
#kde_kcfg_DATA = cervisiapart.kcfg
#
#partrcdir = $(kde_datadir)/cervisiapart
#partrc_DATA = cervisiaui.rc
#
#shellrcdir = $(kde_datadir)/cervisia
#shellrc_DATA = cervisiashellui.rc eventsrc
#
#update_DATA = cervisia.upd
#update_SCRIPTS = move_repositories.pl change_colors.pl cervisia-normalize_cvsroot.pl cervisia-change_repos_list.pl
#updatedir = $(kde_datadir)/kconf_update
#
#messages: rc.cpp
#	$(EXTRACTRC) *.rc >> rc.cpp
#	$(XGETTEXT) -C *.cpp *.h -o $(podir)/cervisia.pot
#
#srcdoc:
#	$(kde_bindir)/kdoc -a -p -d classdocs -n 'Cervisia' *.h -lqt -lkdecore -lkdeui -lkparts
#
#cervisia.1: $(srcdir)/cervisia.1.in
#	sed -e 's%_KDEHTMLDIR_%'${kde_htmldir}'%g;' \
#	    -e 's%_KDECONFDIR_%'${kde_confdir}'%g;' \
#	  < $(srcdir)/cervisia.1.in > cervisia.1
#
#cervisia.1.in: cervisia.pod
#	pod2man --center "Cervisia" --release "${CERVISIA_VERSION}" \
#		cervisia.pod > cervisia.1.in

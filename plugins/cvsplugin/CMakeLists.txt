#INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/cervisia ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(cvsplugin_PART_SRCS
cvsplugin.cpp
dirignorelist.cpp
globalignorelist.cpp
cvscommandbase.cpp
addcommand.cpp
addwatchcommand.cpp
annotatecommand.cpp
commitcommand.cpp
createtagcommand.cpp
deletetagcommand.cpp
editcommand.cpp
fetchbranchtagscommand.cpp
lockcommand.cpp
logcommand.cpp
removecommand.cpp
removewatchcommand.cpp
updatecommand.cpp
updatetagcommand.cpp
diffcommand.cpp
createpatchcommand.cpp
checkoutcommand.cpp
cvs_log_parser.cpp
cvs_annotate_parser.cpp
cvs_update_parser.cpp
checkoutwidget.cpp
)

KDE3_AUTOMOC(${cvsplugin_PART_SRCS})

SET( cvsplugin_DCOP_SKEL_SRCS
cvscommandbase.h
)

KDE3_ADD_DCOP_SKELS(cvsplugin_PART_SRCS ${cvsplugin_DCOP_SKEL_SRCS})

SET( cvsplugin_KCFG_SRCS
cvspluginsettings.kcfgc
)

KDE3_ADD_KCFG_FILES(cvsplugin_PART_SRCS ${cvsplugin_KCFG_SRCS})

KDE3_ADD_KPART(cvsplugin WITH_PREFIX ${cvsplugin_PART_SRCS})

TARGET_LINK_LIBRARIES(cvsplugin  ${QT_AND_KDECORE_LIBS} kparts cervisialib cvsservicestubs )

INSTALL_TARGETS(/lib/kde3 cvsplugin )


########### install files ###############

INSTALL_FILES( /share/apps/cervisiapart/kpartplugins FILES cvsplugin.desktop cvsplugin.rc )
INSTALL_FILES( /share/config.kcfg FILES cvsplugin.kcfg )

#KDE3_PLACEHOLDER()



#original Makefile.am contents follow:

#INCLUDES    = -I$(top_srcdir)/cervisia -I$(top_builddir)/cervisia -I../../cvsservice $(all_includes)
#METASOURCES = AUTO
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = libcvsplugin.la
#
#libcvsplugin_la_SOURCES = cvsplugin.cpp dirignorelist.cpp globalignorelist.cpp \
#                          cvscommandbase.cpp cvscommandbase.skel addcommand.cpp \
#                          addwatchcommand.cpp annotatecommand.cpp commitcommand.cpp \
#                          createtagcommand.cpp deletetagcommand.cpp editcommand.cpp lockcommand.cpp \
#                          logcommand.cpp remotelogcommand.cpp removecommand.cpp removewatchcommand.cpp \
#                          updatecommand.cpp updatetagcommand.cpp diffcommand.cpp \
#                          createpatchcommand.cpp checkoutcommand.cpp \
#                          cvs_log_parser.cpp cvs_annotate_parser.cpp cvs_update_parser.cpp \
#                          cvspluginsettings.kcfgc checkoutwidget.cpp
##libcvsplugin_la_LIBADD  = $(LIB_KPARTS) $(top_builddir)/cervisia/libcervisia.la
#libcvsplugin_la_LIBADD  = $(LIB_KPARTS) $(LIB_KUTILS) $(top_builddir)/cervisia/libcervisia.la \
#                          $(top_builddir)/cervisia/cvsservice/libcvsservice.la
#libcvsplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#pluginsdir = $(kde_datadir)/cervisiapart/kpartplugins
#plugins_DATA = cvsplugin.desktop cvsplugin.rc
#
#kde_kcfg_DATA = cvsplugin.kcfg
#
## NEEDED!
##messages: rc.cpp
##        $(XGETTEXT) *.cpp *.h -o $(podir)/webarchiver.pot

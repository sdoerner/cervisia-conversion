#INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/cervisia ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(svnplugin_PART_SRCS
svnplugin.cpp
globalignorelist.cpp
svncommandbase.cpp
addcommand.cpp
annotatecommand.cpp
commitcommand.cpp
logcommand.cpp
removecommand.cpp
updatecommand.cpp
svn_annotate_parser.cpp
svn_log_parser.cpp
svn_update_parser.cpp
diffcommand.cpp
)

KDE3_AUTOMOC(${svnplugin_PART_SRCS})

SET( svnplugin_DCOP_SKEL_SRCS
svncommandbase.h
)

KDE3_ADD_DCOP_SKELS(svnplugin_PART_SRCS ${svnplugin_DCOP_SKEL_SRCS})

SET( svnplugin_KCFG_SRCS
svnpluginsettings.kcfgc
)

KDE3_ADD_KCFG_FILES(svnplugin_PART_SRCS ${svnplugin_KCFG_SRCS})

KDE3_ADD_KPART(svnplugin WITH_PREFIX ${svnplugin_PART_SRCS})

TARGET_LINK_LIBRARIES(svnplugin  ${QT_AND_KDECORE_LIBS} kparts cervisialib svnservicestubs )

INSTALL_TARGETS(/lib/kde3 svnplugin )


########### install files ###############

INSTALL_FILES( /share/apps/cervisiapart/kpartplugins FILES svnplugin.desktop svnplugin.rc )
INSTALL_FILES( /share/config.kcfg FILES svnplugin.kcfg )

#KDE3_PLACEHOLDER()



#original Makefile.am contents follow:

#INCLUDES    = -I$(top_srcdir)/cervisia -I$(top_builddir)/cervisia -I../../svnservice $(all_includes)
#METASOURCES = AUTO
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = libsvnplugin.la
#
#libsvnplugin_la_SOURCES = svnplugin.cpp globalignorelist.cpp svnpluginsettings.kcfgc \
#                          svncommandbase.cpp svncommandbase.skel addcommand.cpp \
#                          annotatecommand.cpp commitcommand.cpp logcommand.cpp removecommand.cpp \
#                          updatecommand.cpp svn_annotate_parser.cpp svn_log_parser.cpp \
#                          svn_update_parser.cpp diffcommand.cpp
#libsvnplugin_la_LIBADD  = $(LIB_KPARTS) $(LIB_KUTILS) $(top_builddir)/cervisia/libcervisia.la \
#                          $(top_builddir)/cervisia/svnservice/libsvnservice.la
#libsvnplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#pluginsdir = $(kde_datadir)/cervisiapart/kpartplugins
#plugins_DATA = svnplugin.desktop svnplugin.rc
#
#kde_kcfg_DATA = svnplugin.kcfg
#
## NEEDED!
##messages: rc.cpp
##        $(XGETTEXT) *.cpp *.h -o $(podir)/webarchiver.pot

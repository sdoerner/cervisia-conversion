INCLUDES    = -I$(top_srcdir)/cervisia -I$(top_builddir)/cervisia -I../../cvsservice $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = libcvsplugin.la

libcvsplugin_la_SOURCES = cvsplugin.cpp dirignorelist.cpp globalignorelist.cpp \
                          cvscommandbase.cpp cvscommandbase.skel addcommand.cpp \
                          addwatchcommand.cpp annotatecommand.cpp commitcommand.cpp \
                          createtagcommand.cpp editcommand.cpp lockcommand.cpp \
                          logcommand.cpp removecommand.cpp removewatchcommand.cpp \
                          updatecommand.cpp updatetagcommand.cpp \
                          cvs_log_parser.cpp cvs_annotate_parser.cpp cvs_update_parser.cpp \
                          cvspluginsettings.kcfgc
#libcvsplugin_la_LIBADD  = $(LIB_KPARTS) $(top_builddir)/cervisia/libcervisia.la
libcvsplugin_la_LIBADD  = $(LIB_KPARTS) $(LIB_KUTILS) $(top_builddir)/cervisia/libcervisia.la \
                          $(top_builddir)/cervisia/cvsservice/libcvsservice.la
libcvsplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

pluginsdir = $(kde_datadir)/cervisiapart/kpartplugins
plugins_DATA = cvsplugin.desktop cvsplugin.rc

kde_kcfg_DATA = cvsplugin.kcfg

# NEEDED!
#messages: rc.cpp
#        $(XGETTEXT) *.cpp *.h -o $(podir)/webarchiver.pot

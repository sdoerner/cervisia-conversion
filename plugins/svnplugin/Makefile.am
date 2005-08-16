INCLUDES    = -I$(top_srcdir)/cervisia -I$(top_builddir)/cervisia -I../../svnservice $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = libsvnplugin.la

libsvnplugin_la_SOURCES = svnplugin.cpp globalignorelist.cpp \
                          svncommandbase.cpp svncommandbase.skel addcommand.cpp \
                          annotatecommand.cpp commitcommand.cpp logcommand.cpp removecommand.cpp \
                          updatecommand.cpp svn_annotate_parser.cpp svn_log_parser.cpp \
                          svn_update_parser.cpp
#libsvnplugin_la_LIBADD  = $(LIB_KPARTS) $(top_builddir)/cervisia/libcervisia.la
# libcvsservice.la is HACK for progressdialog.cpp
libsvnplugin_la_LIBADD  = $(LIB_KPARTS) $(LIB_KUTILS) $(top_builddir)/cervisia/libcervisia.la \
                          $(top_builddir)/cervisia/svnservice/libsvnservice.la \
                          $(top_builddir)/cervisia/cvsservice/libcvsservice.la
libsvnplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

pluginsdir = $(kde_datadir)/cervisiapart/kpartplugins
plugins_DATA = svnplugin.desktop svnplugin.rc

# NEEDED!
#messages: rc.cpp
#        $(XGETTEXT) *.cpp *.h -o $(podir)/webarchiver.pot
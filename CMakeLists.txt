#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

project( cervisia )

add_subdirectory( cvsservice )
add_subdirectory( pics )

add_definitions( -D_BSD_SOURCE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/cervisia/cvsservice
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_install_icons( )

install( FILES
    cervisia.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

install( FILES
    cervisiapart.kcfg
  DESTINATION ${KCFG_INSTALL_DIR} )

install( FILES
    cervisiaui.rc
  DESTINATION ${DATA_INSTALL_DIR}/cervisiapart )

install( FILES
    cervisiashellui.rc eventsrc
  DESTINATION ${DATA_INSTALL_DIR}/cervisia )

install( FILES
    cervisia.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

install( PROGRAMS
    move_repositories.pl change_colors.pl
    cervisia-normalize_cvsroot.pl cervisia-change_repos_list.pl
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### cervisia (static) #########################

tde_add_library( cervisia STATIC_PIC AUTOMOC
  SOURCES
    annotatedlg.cpp diffdlg.cpp patchoptiondlg.cpp logdlg.cpp
    progressdlg.cpp progressdlg.skel resolvedlg.cpp resolvedlg_p.cpp
    annotateview.cpp diffview.cpp loglist.cpp logplainview.cpp
    logtree.cpp annotatectl.cpp loginfo.cpp misc.cpp qttableview.cpp
    tooltip.cpp cervisiasettings.kcfgc settingsdlg.cpp
    settingsdlg_advanced.ui
  DEPENDENCIES cvsservice-shared
)


##### libcervisiapart (module) ##################

tde_add_kpart( libcervisiapart AUTOMOC
  SOURCES
    updateview.cpp protocolview.cpp protocolview.skel watchdlg.cpp
    changelogdlg.cpp historydlg.cpp repositorydlg.cpp commitdlg.cpp
    checkoutdlg.cpp updatedlg.cpp tagdlg.cpp mergedlg.cpp cvsdir.cpp
    repositories.cpp cervisiapart.cpp addrepositorydlg.cpp
    addremovedlg.cpp watchersdlg.cpp updateview_items.cpp
    updateview_visitors.cpp entry.cpp entry_status.cpp stringmatcher.cpp
    cvsinitdlg.cpp ignorelistbase.cpp dirignorelist.cpp
    globalignorelist.cpp editwithmenu.cpp logmessageedit.cpp
  LINK cervisia-static cvsservice-shared kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### cervisia (kdeinit) ########################

tde_add_kdeinit_executable( cervisia AUTOMOC
  SOURCES
    main.cpp cervisiashell.cpp
  LINK cervisia-static cvsservice-shared kutils-shared
)


##### install import cmake modules ###############

tde_install_export( )

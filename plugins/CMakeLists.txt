ADD_SUBDIRECTORY( cvsplugin ) 
ADD_SUBDIRECTORY( svnplugin ) 

INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


#KDE3_PLACEHOLDER()



#original Makefile.am contents follow:

#SUBDIRS = cvsplugin svnplugin

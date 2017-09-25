project(livetikz)
cmake_minimum_required(VERSION 3.5)

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE_DIRECTORIES( ${KDE4_INCLUDES} /usr/include/poppler/qt4/ . )

set(livetikz_SRCS
   main.cpp
   mainwindow.cpp
   ZoomImage.cpp
   ZoomScrollImage.cpp
 )

kde4_add_executable(livetikz ${livetikz_SRCS})

target_link_libraries(livetikz ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} poppler-qt4)

########### install files ###############
install(TARGETS livetikz DESTINATION ${BIN_INSTALL_DIR} )
install(FILES  
    DESTINATION  ${DATA_INSTALL_DIR}/livetikz ) 

project(livetikz)

cmake_minimum_required(VERSION 3.5)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")
set(CMAKE_AUTORCC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# include KF5 directories
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

# include poppler
include(FindPoppler)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core    # QCommandLineParser, QStringLiteral
    Widgets # QApplication
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons      # KAboutData
    I18n            # KLocalizedString
    XmlGui          # KXmlGuiWindow
    TextEditor      # KTextEdit
    Parts           # Parts
)

find_package(Poppler REQUIRED COMPONENTS Qt5)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(livetikz_SRCS
   main.cpp
   mainwindow.cpp
   ZoomImage.cpp
   ZoomScrollImage.cpp
)

add_executable(livetikz ${livetikz_SRCS} resource.qrc)

target_link_libraries(livetikz
  Qt5::Widgets
  KF5::CoreAddons
  KF5::I18n
  KF5::XmlGui
  KF5::TextEditor
  KF5::Parts
  Poppler::Qt5
)

########### install files ###############
set(LIVETIKZ_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/livetikz)
set(LIVETIKZ_TEMPLATES_INSTALL_DIR ${LIVETIKZ_DATA_INSTALL_DIR}/templates)

install(TARGETS livetikz DESTINATION ${BIN_INSTALL_DIR} )
install(FILES DESTINATION ${DATA_INSTALL_DIR}/livetikz)
install(FILES data/livetikz.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES data/templates/template_example.pgs
	            data/templates/template_beamer.pgs
        DESTINATION ${LIVETIKZ_TEMPLATES_INSTALL_DIR})

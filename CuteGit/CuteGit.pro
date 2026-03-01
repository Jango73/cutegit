
QT += core gui qml quick quickwidgets quickcontrols2 network xml

CONFIG += lrelease
CONFIG += lrelease embed_translations
PRE_TARGETDEPS += compiler_lrelease_make_all

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
DEFINES += "VERSION_STRING=\"\\\"1.0.1\\\"\""
DEFINES += QTPLUS_LIBRARY

# Dependencies
INCLUDEPATH += $$PWD/../qt-plus/source/cpp

# Qt-plus library configuration
QT_PLUS_DIR = $$PWD/../qt-plus
CONFIG(debug, debug|release) {
    QT_PLUS_LIB_DIR = $$QT_PLUS_DIR/build-debug/bin
    QT_PLUS_LIB = -lqt-plusd
} else {
    QT_PLUS_LIB_DIR = $$QT_PLUS_DIR/build-release/bin
    QT_PLUS_LIB = -lqt-plus
}
LIBS += -L$$QT_PLUS_LIB_DIR $$QT_PLUS_LIB

# Sources
include(CuteGit.pri)

# Functions
include($$PWD/../qt-plus/functions.pri)

# Directories
DESTDIR = $$OUT_PWD/bin

# Target binary name
CONFIG(debug, debug|release) {
    unix {
        TARGET = CuteGitApp-dbg
    } else {
        TARGET = CuteGit-dbg
    }
} else {
    unix {
        TARGET = CuteGitApp
    } else {
        TARGET = CuteGit
    }
}

QT_BASE_PATH = $$getQtPath()
QT_BIN_PATH = $$getQtBinPath()
QT_LIB_PATH = $$getQtLibPath()

# Deployment
# In order to activate deployment, add "deploy=1" to qmake arguments

!isEmpty(deploy) {
    message("Deployment files will be copied after linkage, from $${QT_BASE_PATH}.")

    QMAKE_POST_LINK += $$copyFilesWithPathToDir($$QT_LIB_PATH, $$QT_LIB_NAMES, $$DESTDIR)
    QMAKE_POST_LINK += $$copyFilesWithPathToDir($$QT_BASE_PATH, $$QT_PLUGIN_NAMES, $$DESTDIR)
    QMAKE_POST_LINK += $$copyFilesToDir($$PWD, $$DEPLOY_NAMES, $$DESTDIR)
    QMAKE_POST_LINK += $$copyDirsToDir($$QT_BASE_PATH, $$QT_QML_NAMES, $$DESTDIR)

    unix {
        QMAKE_POST_LINK += $$makeExecutable($$EXEC_NAMES, $$DESTDIR)
    }
}

# Installer - WIP
# In order to activate installer creation, add "installer=1" to qmake arguments
#!isEmpty(deploy) {
#    !isEmpty(installer) {
#        installerBin = $$QT_BIN_PATH/binarycreator.exe
#        packageDirectory = $$DESTDIR
#        configFile = $$PWD/deploy/installer-config.xml
#        cmd = $$installerBin --offline-only -p $$packageDirectory -c $$configFile $${TARGET}Installer
#    }
#}

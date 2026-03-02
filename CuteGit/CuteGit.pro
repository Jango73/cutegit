
QT += core gui qml quick quickwidgets quickcontrols2 network xml

# CONFIG += lrelease
# CONFIG += lrelease embed_translations
# PRE_TARGETDEPS += compiler_lrelease_make_all
# QMAKE_LRELEASE = /usr/bin/lrelease

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

QT_BASE_PATH = $$[QT_INSTALL_PREFIX]
QT_BIN_PATH = $$[QT_INSTALL_BINS]
QT_LIB_PATH = $$[QT_INSTALL_LIBS]
QT_PLUGIN_PATH = $$[QT_INSTALL_PLUGINS]
QT_QML_PATH = $$[QT_INSTALL_QML]

# Deployment
# In order to activate deployment, add "deploy=1" to qmake arguments

!isEmpty(deploy) {
    message("Deployment files will be copied after linkage, from $${QT_BASE_PATH}.")

    # Copy library files preserving path
    for(lib, QT_LIB_NAMES) {
        src = $$QT_LIB_PATH/$$lib
        dst = $$DESTDIR/$$lib
        dst_dir = $$system_path($$dst)
        dst_dir = $$replace(dst_dir, /[^/]*$, )
        QMAKE_POST_LINK += mkdir -p $$shell_quote($$dst_dir) && cp -r $$shell_quote($$system_path($$src)) $$shell_quote($$system_path($$dst))
    }

    # Copy plugin files preserving path
    for(plugin, QT_PLUGIN_NAMES) {
        plugin_rel = $$section(plugin, /, 1)
        src = $$QT_PLUGIN_PATH/$$plugin_rel
        dst = $$DESTDIR/$$plugin
        dst_dir = $$system_path($$dst)
        dst_dir = $$replace(dst_dir, /[^/]*$, )
        QMAKE_POST_LINK += mkdir -p $$shell_quote($$dst_dir) && cp -r $$shell_quote($$system_path($$src)) $$shell_quote($$system_path($$dst))
    }

    # Copy deployment files (flat)
    for(file, DEPLOY_NAMES) {
        src = $$PWD/$$file
        dst = $$DESTDIR/$$section(file, /, -1, -1)
        QMAKE_POST_LINK += cp $$shell_quote($$system_path($$src)) $$shell_quote($$system_path($$dst))
    }

    # Copy QML directories recursively
    for(dir, QT_QML_NAMES) {
        qml_rel = $$section(dir, /, 1)
        src = $$QT_QML_PATH/$$qml_rel
        dst = $$DESTDIR/$$dir
        QMAKE_POST_LINK += mkdir -p $$shell_quote($$system_path($$dst)) && cp -r $$shell_quote($$system_path($$src))/* $$shell_quote($$system_path($$dst))/
    }

    unix {
        for(exec, EXEC_NAMES) {
            QMAKE_POST_LINK += chmod +x $$shell_quote($$system_path($$DESTDIR/$$exec))
        }
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

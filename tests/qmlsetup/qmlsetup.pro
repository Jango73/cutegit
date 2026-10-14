QT += core gui qml quick testlib qmltest xml
QT -= widgets

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = diff-qmlsetup

# Section binary folder: tests/bin
DESTDIR = $$PWD/../bin

OBJECTS_DIR = $$OUT_PWD/obj
MOC_DIR = $$OUT_PWD/moc
RCC_DIR = $$OUT_PWD/rcc
UI_DIR = $$OUT_PWD/ui

CUTEGIT_SOURCES = $$PWD/../../CuteGit/sources
QTPLUS_SOURCES = $$PWD/../../qt-plus/source/cpp

INCLUDEPATH += $$CUTEGIT_SOURCES $$QTPLUS_SOURCES
DEPENDPATH += $$CUTEGIT_SOURCES

SOURCES += \
    setup.cpp \
    $$CUTEGIT_SOURCES/CPatchBuilder.cpp \
    $$CUTEGIT_SOURCES/CDiffModel.cpp \
    $$CUTEGIT_SOURCES/CDiffModelProxy.cpp

HEADERS += \
    $$CUTEGIT_SOURCES/CPatchBuilder.h \
    $$CUTEGIT_SOURCES/CDiffLine.h \
    $$CUTEGIT_SOURCES/CDiffModel.h \
    $$CUTEGIT_SOURCES/CDiffModelProxy.h \
    $$CUTEGIT_SOURCES/CEnums.h

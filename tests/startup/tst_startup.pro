QT += core gui widgets testlib xml qml quick quickcontrols2
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_startup

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

# Same resources as production: App.qml, images, layouts
RESOURCES += $$PWD/../../CuteGit/resources.qrc

DEFINES += "VERSION_STRING=\"\\\"1.0.1\\\"\""

SOURCES += \
    tst_startup.cpp \
    $$CUTEGIT_SOURCES/CController.cpp \
    $$CUTEGIT_SOURCES/CLayout.cpp \
    $$CUTEGIT_SOURCES/CRepositoryModel.cpp \
    $$CUTEGIT_SOURCES/CRepository.cpp \
    $$CUTEGIT_SOURCES/CRepositoryDiff.cpp \
    $$CUTEGIT_SOURCES/CRepositoryOutput.cpp \
    $$CUTEGIT_SOURCES/CPatchBuilder.cpp \
    $$CUTEGIT_SOURCES/CDiffConstants.cpp \
    $$CUTEGIT_SOURCES/CDiffModel.cpp \
    $$CUTEGIT_SOURCES/CDiffModelProxy.cpp \
    $$CUTEGIT_SOURCES/CFlatFileModel.cpp \
    $$CUTEGIT_SOURCES/CFlatFileModelProxy.cpp \
    $$CUTEGIT_SOURCES/CStagedFileModelProxy.cpp \
    $$CUTEGIT_SOURCES/CBranchModel.cpp \
    $$CUTEGIT_SOURCES/CLogModel.cpp \
    $$CUTEGIT_SOURCES/CLogModelProxy.cpp \
    $$CUTEGIT_SOURCES/CLogLineCollection.cpp \
    $$CUTEGIT_SOURCES/CGraphModel.cpp \
    $$CUTEGIT_SOURCES/CGraphLine.cpp \
    $$CUTEGIT_SOURCES/CGraphLineCollection.cpp \
    $$CUTEGIT_SOURCES/CGraphLayout.cpp \
    $$CUTEGIT_SOURCES/CRepoFile.cpp \
    $$CUTEGIT_SOURCES/CLabelModel.cpp \
    $$CUTEGIT_SOURCES/CUtils.cpp \
    $$CUTEGIT_SOURCES/Strings.cpp \
    $$CUTEGIT_SOURCES/commands/CCommands.cpp \
    $$CUTEGIT_SOURCES/commands/CGitCommands.cpp \
    $$CUTEGIT_SOURCES/commands/CGerritCommands.cpp \
    $$CUTEGIT_SOURCES/commands/CSvnCommands.cpp \
    $$CUTEGIT_SOURCES/commands/CHgCommands.cpp \
    $$CUTEGIT_SOURCES/commands/CExecution.cpp \
    $$QTPLUS_SOURCES/CXMLNode.cpp

HEADERS += \
    $$CUTEGIT_SOURCES/Strings.h \
    $$CUTEGIT_SOURCES/Macros.h \
    $$CUTEGIT_SOURCES/CController.h \
    $$CUTEGIT_SOURCES/CLayout.h \
    $$CUTEGIT_SOURCES/CRepositoryModel.h \
    $$CUTEGIT_SOURCES/CRepository.h \
    $$CUTEGIT_SOURCES/CPatchBuilder.h \
    $$CUTEGIT_SOURCES/CDiffConstants.h \
    $$CUTEGIT_SOURCES/CDiffLine.h \
    $$CUTEGIT_SOURCES/CDiffModel.h \
    $$CUTEGIT_SOURCES/CDiffModelProxy.h \
    $$CUTEGIT_SOURCES/CFlatFileModel.h \
    $$CUTEGIT_SOURCES/CFlatFileModelProxy.h \
    $$CUTEGIT_SOURCES/CStagedFileModelProxy.h \
    $$CUTEGIT_SOURCES/CBranch.h \
    $$CUTEGIT_SOURCES/CBranchModel.h \
    $$CUTEGIT_SOURCES/CLogModel.h \
    $$CUTEGIT_SOURCES/CLogModelProxy.h \
    $$CUTEGIT_SOURCES/CLogLine.h \
    $$CUTEGIT_SOURCES/CLogLineCollection.h \
    $$CUTEGIT_SOURCES/CGraphModel.h \
    $$CUTEGIT_SOURCES/CGraphLine.h \
    $$CUTEGIT_SOURCES/CGraphLineCollection.h \
    $$CUTEGIT_SOURCES/CRepoFile.h \
    $$CUTEGIT_SOURCES/CLabel.h \
    $$CUTEGIT_SOURCES/CLabelModel.h \
    $$CUTEGIT_SOURCES/CEnums.h \
    $$CUTEGIT_SOURCES/commands/CCommands.h \
    $$CUTEGIT_SOURCES/commands/CGitCommands.h \
    $$CUTEGIT_SOURCES/commands/CGerritCommands.h \
    $$CUTEGIT_SOURCES/commands/CSvnCommands.h \
    $$CUTEGIT_SOURCES/commands/CHgCommands.h \
    $$CUTEGIT_SOURCES/commands/CExecution.h

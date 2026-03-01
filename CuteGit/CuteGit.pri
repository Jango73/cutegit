
HEADERS += \
    #../qt-plus/source/cpp/CXMLNode.h \
    sources/CBranch.h \
    sources/CBranchModel.h \
    sources/CController.h \
    sources/CDiffLine.h \
    sources/CDiffModel.h \
    sources/CDiffModelProxy.h \
    sources/CEnums.h \
    sources/CFlatFileModel.h \
    sources/CFlatFileModelProxy.h \
    sources/CGraphLine.h \
    sources/CGraphModel.h \
    sources/CLabel.h \
    sources/CLabelModel.h \
    sources/CLogLine.h \
    sources/CLogLineCollection.h \
    sources/CLogModel.h \
    sources/CLogModelProxy.h \
    sources/CRepoFile.h \
    sources/CRepository.h \
    sources/CRepositoryModel.h \
    sources/CStagedFileModelProxy.h \
    sources/CTreeFileModel.h \
    sources/CTreeFileModelProxy.h \
    sources/CUtils.h \
    sources/CuteGit.h \
    sources/Strings.h \
    sources/commands/CCommands.h \
    sources/commands/CExecution.h \
    sources/commands/CGerritCommands.h \
    sources/commands/CGitCommands.h \
    sources/commands/CHgCommands.h \
    sources/commands/CSvnCommands.h \

SOURCES += \
    #../qt-plus/source/cpp/CXMLNode.cpp \
    sources/CBranchModel.cpp \
    sources/CController.cpp \
    sources/CDiffModel.cpp \
    sources/CDiffModelProxy.cpp \
    sources/CFlatFileModel.cpp \
    sources/CFlatFileModelProxy.cpp \
    sources/CGraphModel.cpp \
    sources/CLabelModel.cpp \
    sources/CLogLineCollection.cpp \
    sources/CLogModel.cpp \
    sources/CLogModelProxy.cpp \
    sources/CRepoFile.cpp \
    sources/CRepository.cpp \
    sources/CRepositoryModel.cpp \
    sources/CStagedFileModelProxy.cpp \
    sources/CTreeFileModel.cpp \
    sources/CTreeFileModelProxy.cpp \
    sources/CUtils.cpp \
    sources/CuteGit.cpp \
    sources/Strings.cpp \
    sources/commands/CCommands.cpp \
    sources/commands/CExecution.cpp \
    sources/commands/CGerritCommands.cpp \
    sources/commands/CGitCommands.cpp \
    sources/commands/CHgCommands.cpp \
    sources/commands/CSvnCommands.cpp \
    sources/main.cpp \

RESOURCES += \
    resources.qrc

TRANSLATIONS = \
    i18n/CuteGit_fr.ts \
    i18n/CuteGit_de.ts \
    i18n/CuteGit_es.ts \
    i18n/CuteGit_ja.ts \
    i18n/CuteGit_zh.ts \

unix {
    QT_LIB_NAMES = \
        libicudata.so \
        libicui18n.so \
        libicuuc.so \
        libQt6Core.so.6 \
        libQt6DBus.so.6 \
        libQt6Gui.so.6 \
        libQt6Network.so.6 \
        libQt6OpenGL.so.6 \
        libQt6Qml.so.6 \
        libQt6Quick.so.6 \
        libQt6QuickControls2.so.6 \
        libQt6QuickTemplates2.so.6 \
        libQt6QuickWidgets.so.6 \
        libQt6Svg.so.6 \
        libQt6Widgets.so.6 \
        libQt6XcbQpa.so.6 \
        libQt6Xml.so.6

    QT_PLUGIN_NAMES = \
        plugins/imageformats/libqico.so \
        plugins/imageformats/libqjpeg.so \
        plugins/imageformats/libqsvg.so \
        plugins/platforminputcontexts/libcomposeplatforminputcontextplugin.so \
        plugins/platforms/libqxcb.so \
        plugins/xcbglintegrations/libqxcb-egl-integration.so \
        plugins/xcbglintegrations/libqxcb-glx-integration.so

    DEPLOY_NAMES = \
        deploy/linux/qt.conf \
        deploy/linux/CuteGit.sh

    EXEC_NAMES = \
        CuteGit.sh
} else {
    QT_LIB_NAMES = \
        libgcc_s_seh-1.dll \
#        libstdc++-6.dll \
        libwinpthread-1.dll \
        Qt6Core.dll \
        Qt6DBus.dll \
        Qt6Gui.dll \
        Qt6Network.dll \
        Qt6OpenGL.dll \
        Qt6Qml.dll \
        Qt6Quick.dll \
        Qt6QuickControls2.dll \
        Qt6QuickTemplates2.dll \
        Qt6QuickWidgets.dll \
        Qt6Svg.dll \
        Qt6Widgets.dll \
        Qt6Xml.dll

    QT_PLUGIN_NAMES = \
        plugins/imageformats/qico.dll \
        plugins/imageformats/qjpeg.dll \
        plugins/imageformats/qsvg.dll \
        plugins/platforms/qwindows.dll

    DEPLOY_NAMES = \
        deploy/linux/qt.conf

    EXEC_NAMES =
}

QT_QML_NAMES = \
    qml/Qt/labs/folderlistmodel \
    qml/Qt/labs/settings \
    qml/Qt/labs/platform \
    qml/QtQml/Models.2 \
    qml/QtQuick/Controls \
    qml/QtQuick/Controls.2 \
    qml/QtQuick/Dialogs \
    qml/QtQuick/Layouts \
    qml/QtQuick/PrivateWidgets \
    qml/QtQuick/Scene2D \
    qml/QtQuick/Templates.2 \
    qml/QtQuick/Window.2 \
    qml/QtQuick/XmlListModel \
    qml/QtQuick.2

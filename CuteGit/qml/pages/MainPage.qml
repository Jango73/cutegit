import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.5 as QC15
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.2
import Qt.labs.platform 1.1 as QLP
import CuteGit 1.0
import "../js/Utils.js" as Utils
import "../components"
import "../views"
import "../popups"

Item {
    id: root

    property variant controller: null
    property variant materialTheme: 0

    signal requestDarkTheme()
    signal requestLightTheme()

    MainMenu {
        id: menu
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        controller: root.controller
        repository: root.controller.currentRepository
        materialTheme: root.materialTheme

        onRequestCloneRepository: cloneDialog.open()
        onRequestOpenRepository: openDialog.open()
        onRequestDarkTheme: root.requestDarkTheme()
        onRequestLightTheme: root.requestLightTheme()
        onRequestHelp: helpDialog.open()

        Component.onCompleted: {
            repositoryView = Qt.binding(function() {
                return container.count > 0
                        ? container.getTab(container.currentIndex).item
                        : null
            })
        }
    }

    StandardToolBar {
        id: toolBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: menu.bottom

        Row {
            spacing: Const.mainPadding

            StandardToolButton {
                action: menu.cloneRepositoryAction
                icon.source: Const.cloneIcon
            }

            StandardToolButton {
                action: menu.openRepositoryAction
                icon.source: Const.openIcon
            }

            ToolSeparator {}

            StandardToolButton {
                action: menu.fetchAction
                icon.source: Const.fetchIcon
            }

            StandardToolButton { action: menu.pullAction }
            StandardToolButton { action: menu.pushAction }

            ToolSeparator {}

            StandardToolButton {
                action: menu.stageSelectionAction
                text: Const.stageText
                icon.source: Const.stageIcon
            }

            StandardToolButton {
                action: menu.revertSelectionAction
                text: Const.revertText
                icon.source: Const.revertIcon
            }

            ToolSeparator {}

            StandardToolButton {
                action: menu.commitAction
                icon.source: Const.commitIcon
            }

            StandardToolButton {
                action: menu.amendAction
                icon.source: Const.commitIcon
            }

            ToolSeparator {}

            StandardToolButton {
                action: menu.saveStashAction
                icon.source: Const.saveStashIcon
            }

            StandardToolButton {
                action: menu.popStashAction
                icon.source: Const.popStashIcon
            }
        }
    }

    Item {
        id: clientZone
        anchors.top: toolBar.bottom
        anchors.bottom: statusBar.top
        anchors.left: parent.left
        anchors.right: parent.right

        QC15.TabView {
            id: container
            anchors.fill: parent

            style: StandardTabViewStyle {
                canClose: true
                closeAction: tabCloseAction
            }

            Action {
                id: tabCloseAction

                onTriggered: {
                    root.controller.removeRepository(source.index)
                    container.removeTab(source.index)
                }
            }

            Repeater {
                model: root.controller.openRepositoryModel

                QC15.Tab {
                    title: model.repository.repositoryName + " - " + model.repository.repositoryTypeString

                    RepositoryView {
                        repository: model.repository
                        filesAsTree: menu.filesAsTree
                    }
                }
            }

            onCountChanged: {
                var index = root.controller.currentRepositoryIndexToSet()
                if (index !== -1)
                    currentIndex = index

                root.controller.currentRepositoryIndex = currentIndex
            }

            onCurrentIndexChanged: {
                root.controller.currentRepositoryIndex = currentIndex
            }
        }

        StandardLabel {
            anchors.fill: parent
            visible: root.controller.openRepositoryModel.count === 0
            text: Const.emptyRepositoryTabText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Item {
        id: statusBar
        anchors.bottom: parent.bottom
        width: parent.width
        height: Const.elementHeight

        RowLayout {
            anchors.fill: parent
            StandardLabel {
                text: root.controller.statusText
            }
        }
    }

    ClonePopup {
        id: cloneDialog
        width: root.width * Const.popupWidthNorm
        height: root.height * Const.popupHeightSmall
        anchors.centerIn: parent

        controller: root.controller

        onCloneBegins: statusTextHistory.open()
    }

    QLP.FolderDialog {
        id: openDialog

        onAccepted: {
            root.controller.openRepository(folder)
        }
    }

    StandardPopup {
        id: helpDialog
        width: root.width * Const.popupWidthNorm
        height: root.height * Const.popupHeightNorm
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape

        StandardText {
            id: title
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Const.mainPadding

            horizontalAlignment: Text.AlignHCenter
            text: Const.helpTitleText
        }

        StandardText {
            id: helpText
            anchors.top: title.bottom
            anchors.bottom: parent.bottom
            width: parent.width
        }

        Component.onCompleted: {
            helpText.text = Const.helpText.format(root.controller.version)
        }
    }

    Popup {
        id: statusTextHistory
        width: root.width * Const.popupWidthNorm
        height: root.height * Const.popupHeightNorm
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape
        padding: Const.mainPadding

        StandardStringListView {
            anchors.fill: parent
            model: root.controller.statusTextHistory
        }
    }
}

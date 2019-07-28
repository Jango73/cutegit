import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "../components"

TitlePane {
    id: root

    property variant controller: null
    property string branchName: ""

    signal requestDeleteBranch(var name)

    title: Const.branchesText

    content: Item {
        anchors.fill: parent

        StandardListView {
            id: list
            anchors.fill: parent
            visible: count > 0

            model: root.controller.repository.branchModel

            delegate: Item {
                width: parent.width
                height: Const.elementHeight

                Item {
                    anchors.fill: parent
                    anchors.margins: Const.smallPadding

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.AllButtons
                        onClicked: {
                            if (mouse.button === Qt.RightButton) {
                                root.branchName = model.name
                                menu.popup()
                            }
                        }
                        onDoubleClicked: {
                            root.controller.repository.currentBranch = model.name
                        }
                    }

                    Selection {
                        id: selection
                        targetWidth: text.width
                        targetHeight: text.height
                        anchors.centerIn: text
                        visible: model.name === root.controller.repository.currentBranch
                    }

                    ElideText {
                        id: text
                        width: parent.width - Const.smallPadding
                        text: model.name
                        color: selection.visible ? Material.background : Material.foreground
                    }
                }
            }
        }

        BranchMenu {
            id: menu

            onRequestDeleteBranch: {
                root.requestDeleteBranch(root.branchName)
            }
        }
    }
}

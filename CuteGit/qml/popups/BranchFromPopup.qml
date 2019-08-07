import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

StandardPopup {
    id: root
    modal: true
    closePolicy: Popup.CloseOnEscape
    padding: Const.mainPadding

    Material.elevation: Const.popupElevation

    property variant repository: null
    property string commitId: ""

    Component.onCompleted: {
        root.forceActiveFocus()
    }

    contentItem: Item {
        StandardText {
            id: title
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Const.mainPadding

            horizontalAlignment: Text.AlignHCenter
            text: Const.branchFromText
        }

        StandardTextEdit {
            id: name
            anchors.top: title.bottom
            anchors.bottom: buttons.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Const.mainPadding

            placeHolderText: Const.enterBranchNameHereText
            focus: true
        }

        StandardToolBar {
            id: buttons
            width: parent.width
            height: cancelButton.height + Const.mainPadding
            anchors.bottom: parent.bottom

            Row {
                spacing: Const.mainPadding

                StandardToolButton {
                    action: Action {
                        id: okButton
                        text: Const.okText
                        onTriggered: {
                            root.close()
                            root.repository.commitBranchFrom(root.commitId, name.text)
                        }
                    }
                }

                StandardToolButton {
                    action: Action {
                        id: cancelButton
                        text: Const.cancelText
                        onTriggered: {
                            root.close()
                        }
                    }
                }
            }
        }
    }
}

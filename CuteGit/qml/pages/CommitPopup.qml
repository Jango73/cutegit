import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

Popup {
    id: root
    modal: true
    closePolicy: Popup.CloseOnEscape
    padding: Const.mainPadding

    Material.elevation: Const.popupElevation

    property variant controller: null
    property bool amend: false

    Component.onCompleted: {
        root.forceActiveFocus()
    }

    contentItem: Item {
        anchors.fill: parent

        StandardText {
            id: title
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Const.mainPadding

            horizontalAlignment: Text.AlignHCenter
            text: root.amend ? qsTr("Amend") : qsTr("Commit")
        }

        StandardTextEdit {
            id: message
            anchors.top: title.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height * 0.4
            anchors.margins: Const.mainPadding
            focus: true
        }

        StandardText {
            id: fileList
            anchors.top: message.bottom
            anchors.bottom: buttons.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Const.mainPadding
            text: qsTr("File list")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        RowLayout {
            id: buttons
            width: parent.width
            height: cancelButton.height * 1.1
            anchors.bottom: parent.bottom

            StandardButton {
                id: okButton
                Layout.alignment: Qt.AlignCenter
                text: Const.okText

                enabled: message.text != "" || root.controller.fileModel.repositoryStatus !== CFileModel.NoMerge

                onClicked: {
                    root.close()
                    root.controller.fileModelProxy.commit(message.text, root.amend)
                }
            }

            StandardButton {
                id: cancelButton
                Layout.alignment: Qt.AlignCenter
                text: Const.cancelText

                onClicked: {
                    root.close()
                }
            }
        }
    }
}

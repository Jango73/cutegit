import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import CuteGit 1.0
import "../components"

StandardPopup {
    id: root

    property variant controller: null
    property string commitId: ""
    property bool showFileList: true
    property bool amend: false

    property alias messageText: message.text
    property alias messageEnabled: message.enabled

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
            text: root.showFileList ? root.amend ? qsTr("Amend") : qsTr("Commit") : qsTr("Change commit message")
        }

        StandardTextEdit {
            id: message
            anchors.top: title.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Const.mainPadding
            height: parent.height * 0.3

            placeHolderText: Const.enterMessageHereText
            focus: true
        }

        TitlePane {
            id: filePane
            anchors.top: message.bottom
            anchors.bottom: buttons.top
            anchors.left: parent.left
            anchors.right: parent.right

            title: Const.filesText

            content: FlatFileView {
                id: fileList
                anchors.fill: parent
                anchors.margins: Const.mainPadding
                controller: root.controller
                visible: root.showFileList
                mouseActive: false
            }
        }

        RowLayout {
            id: buttons
            width: parent.width
            height: cancelButton.height + Const.mainPadding
            anchors.bottom: parent.bottom

            StandardButton {
                id: okButton
                Layout.alignment: Qt.AlignCenter
                text: Const.okText

                enabled: message.text != "" || root.controller.repository.repositoryStatus !== CEnums.NoMerge

                onClicked: {
                    root.close()

                    if (root.showFileList) {
                        root.controller.repository.commit(message.text, root.amend)
                    } else {
                        root.controller.repository.changeCommitMessage(commitId, message.text)
                    }
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

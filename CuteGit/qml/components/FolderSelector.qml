import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

Item {
    id: root
    anchors.margins: Const.mainPadding
    implicitHeight: title.height + folderName.height

    property alias title: title.text
    property alias text: folderName.text
    property variant controller: null

    StandardLabel {
        id: title
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Item {
        id: content
        anchors.top: title.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        StandardTextField {
            id: folderName
            anchors.left: parent.left
            anchors.right: browseButton.left
            anchors.rightMargin: Const.mainPadding
        }

        StandardButton {
            id: browseButton
            text: Const.browseText
            anchors.right: parent.right
            tipText: Const.tipBrowseButton

            onClicked: {
                if (root.controller !== null) {
                    var sFolder = root.controller.browseForExistingFolder(root.title)
                    if (sFolder !== "")
                        folderName.text = sFolder
                }
            }
        }
    }
}

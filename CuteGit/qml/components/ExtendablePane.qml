import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

TitlePane {
    id: root

    //! When set, the extend button is enabled and the pane fills its visual parent while extended
    property Item extensionParent: null

    StandardButton {
        anchors.top: parent.top
        anchors.right: parent.right
        width: Const.microButtonWidth
        height: Const.microButtonHeight
        padding: 0
        flat: true
        text: root.state === "" ? "^" : "v"
        tipText: Const.tipExpandButton

        onClicked: {
            if (root.extensionParent !== null)
                root.state = (root.state === "" ? "extended" : "")
        }
    }

    states: [
        State {
            name: ""
        },
        State {
            name: "extended"
            AnchorChanges {
                target: root
                anchors.top: root.parent.top
                anchors.bottom: root.parent.bottom
                anchors.left: root.parent.left
                anchors.right: root.parent.right
            }
            PropertyChanges {
                target: root
                z: 999999
            }
        }
    ]
}

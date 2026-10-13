import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "."

TabButton {
    id: root
    implicitHeight: Const.toolButtonHeight
    topPadding: 0
    bottomPadding: 0
    topInset: 0
    bottomInset: 0

    property Action closeAction: null
    property int tabIndex: 0

    contentItem: RowLayout {
        id: row
        implicitHeight: Const.elementHeight

        StandardText {
            height: Const.elementHeight
            font.capitalization: Font.AllUppercase
            text: root.text
            color: root.pressed || root.checked ? Material.accent : Material.foreground
        }

        Item {
            width: height
            height: Const.elementHeight

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    if (root.closeAction !== null)
                        root.closeAction.trigger(root)
                }
            }

            Item {
                id: cross
                anchors.centerIn: parent
                width: parent.width * 0.8
                height: parent.height * 0.8

                Rectangle {
                    x: 0
                    y: parent.height * 0.5 - height * 0.3
                    width: parent.width
                    height: 3
                    color: Material.foreground
                }

                Rectangle {
                    x: parent.width * 0.5 - width * 0.3
                    y: 0
                    width: 3
                    height: parent.height
                    color: Material.foreground
                }

                transform: Rotation {
                    origin.x: cross.width * 0.5
                    origin.y: cross.height * 0.5
                    angle: 45
                }
            }
        }
    }
}

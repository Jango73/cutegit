import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

Rectangle {
    id: root
    color: borderOnly ? Const.transparent : Material.primary
    border.width: 2
    border.color: borderOnly ? Material.primary : Const.transparent
    opacity: 0
    visible: opacity !== 0
    state: show ? "on" : "off"

    property bool show: false
    property bool borderOnly: false

    states: [
        State {
            name: "off"
            PropertyChanges {
                target: root
                opacity: 0
            }
        },
        State {
            name: "on"
            PropertyChanges {
                target: root
                opacity: 1
            }
        }
    ]
}

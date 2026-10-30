import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    CommandsPane {
        id: commandsPane
        anchors.fill: parent
        showTitle: false
        extensionParent: root
        repository: root.repository
    }

    function focusTab() {
        commandsPane.forceActiveFocus()
    }
}

import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    OutputPane {
        id: outputPane
        anchors.fill: parent
        showTitle: false
        extensionParent: root
        repository: root.repository
    }

    function focusTab() {
        outputPane.forceActiveFocus()
    }
}

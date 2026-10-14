import QtQuick 2.12
import "../pages"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

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

import QtQuick 2.12
import "../views"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

    GraphView {
        id: graphView
        anchors.fill: parent

        model: root.repository !== null ? root.repository.graphModel : undefined
    }

    function focusTab() {
        graphView.forceActiveFocus()
    }
}

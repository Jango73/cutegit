import QtQuick 2.12
import "../views"

BaseTab {
    id: root

    GraphView {
        id: graphView
        anchors.fill: parent

        model: root.repository !== null ? root.repository.graphModel : undefined
    }

    function focusTab() {
        graphView.forceActiveFocus()
    }
}

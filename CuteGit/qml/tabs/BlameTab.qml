import QtQuick 2.12
import "../views"

BaseTab {
    id: root

    BlameView {
        id: blameView
        anchors.fill: parent
        repository: root.repository

        onRequestTextFilter: function(text) { if (root.repository !== null) root.repository.setFileBlameFilter(text) }
    }

    function focusTab() {
        blameView.forceActiveFocus()
    }
}

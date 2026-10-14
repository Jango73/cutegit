import QtQuick 2.12
import "../views"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

    BlameView {
        id: blameView
        anchors.fill: parent
        repository: root.repository

        onRequestTextFilter: if (root.repository !== null) root.repository.setFileBlameFilter(text)
    }

    function focusTab() {
        blameView.forceActiveFocus()
    }
}

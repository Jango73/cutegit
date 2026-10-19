import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    CommitDetailsPane {
        id: detailsPane
        anchors.fill: parent
        repository: root.repository
    }

    function focusTab() {
        detailsPane.focusPane()
    }

    function loadData() {
        if (root.repository !== null)
            root.repository.refreshCommitDetails()
    }
}

import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    FileLogPane {
        id: fileLogPane
        anchors.fill: parent
        repository: root.repository

        onRequestMenu: root.viewRoot.showLogMenu(commitId, message)
        onRequestCommitDiffPrevious: root.viewRoot.requestCommitDiffPrevious(commitId)
        onRequestCommitDiffFrom: root.viewRoot.requestCommitDiffFrom(commitId)
        onRequestCommitDiffTo: root.viewRoot.requestCommitDiffTo(commitId)
    }

    function focusTab() {
        fileLogPane.forceActiveFocus()
    }
}

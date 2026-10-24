import QtQuick 2.12
import "../pages"

CommitLogTabBase {
    id: root

    FileLogPane {
        id: fileLogPane
        anchors.fill: parent
        repository: root.repository

        onRequestMenu: root.showCommitMenu(commitId, message)
        onRequestCommitDiffPrevious: root.requestCommitDiffPrevious(commitId)
        onRequestCommitDiffFrom: root.requestCommitDiffFrom(commitId)
        onRequestCommitDiffTo: root.requestCommitDiffTo(commitId)
    }

    function focusTab() {
        fileLogPane.forceActiveFocus()
    }
}

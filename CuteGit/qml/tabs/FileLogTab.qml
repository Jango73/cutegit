import QtQuick 2.12
import "../pages"

CommitLogTabBase {
    id: root

    FileLogPane {
        id: fileLogPane
        anchors.fill: parent
        repository: root.repository

        onRequestMenu: function(commitId, message) { root.showCommitMenu(commitId, message) }
        onRequestCommitDiffPrevious: function(commitId) { root.requestCommitDiffPrevious(commitId) }
        onRequestCommitDiffFrom: function(commitId) { root.requestCommitDiffFrom(commitId) }
        onRequestCommitDiffTo: function(commitId) { root.requestCommitDiffTo(commitId) }
    }

    function focusTab() {
        fileLogPane.forceActiveFocus()
    }
}

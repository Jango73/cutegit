import QtQuick 2.12
import "../views"

CommitLogTabBase {
    id: root

    LogView {
        id: refLogView
        anchors.fill: parent

        model: root.repository !== null ? root.repository.refLogModel : undefined
        diffFromCommitId: root.diffFromCommitId
        diffToCommitId: root.diffToCommitId

        onItemClicked: function(commitId) { root.showCommitDiffPrevious(commitId) }
        onItemRightClicked: function(commitId, message) { root.showCommitMenu(commitId, message, true) }
        onRequestCommitDiffPrevious: function(commitId) { root.requestCommitDiffPrevious(commitId) }
        onRequestCommitDiffFrom: function(commitId) { root.requestCommitDiffFrom(commitId) }
        onRequestCommitDiffTo: function(commitId) { root.requestCommitDiffTo(commitId) }
    }

    function focusTab() {
        refLogView.forceActiveFocus()
    }

    function loadData() {
        if (root.repository !== null)
            root.repository.refreshRefLog()
    }
}

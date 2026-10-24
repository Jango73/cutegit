import QtQuick 2.12
import "../views"

CommitLogTabBase {
    id: root

    GraphView {
        id: graphView
        anchors.fill: parent

        model: root.repository !== null ? root.repository.graphModel : undefined
        diffFromCommitId: root.diffFromCommitId
        diffToCommitId: root.diffToCommitId

        onItemClicked: root.showCommitDiffPrevious(commitId)
        onItemRightClicked: root.showCommitMenu(commitId, message)
        onRequestCommitDiffPrevious: root.requestCommitDiffPrevious(commitId)
        onRequestCommitDiffFrom: root.requestCommitDiffFrom(commitId)
        onRequestCommitDiffTo: root.requestCommitDiffTo(commitId)
    }

    function focusTab() {
        graphView.forceActiveFocus()
    }

    function loadData() {
        if (root.repository !== null)
            root.repository.refreshGraph()
    }
}

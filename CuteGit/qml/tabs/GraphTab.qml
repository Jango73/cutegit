import QtQuick 2.12
import "../views"

BaseTab {
    id: root

    GraphView {
        id: graphView
        anchors.fill: parent

        model: root.repository !== null ? root.repository.graphModel : undefined
        diffFromCommitId: root.repository ? root.repository.diffFromCommitId : ""
        diffToCommitId: root.repository ? root.repository.diffToCommitId : ""

        onItemRightClicked: root.viewRoot.showLogMenu(commitId, message)
        onRequestCommitDiffPrevious: root.viewRoot.requestCommitDiffPrevious(commitId)
        onRequestCommitDiffFrom: root.viewRoot.requestCommitDiffFrom(commitId)
        onRequestCommitDiffTo: root.viewRoot.requestCommitDiffTo(commitId)
    }

    function focusTab() {
        graphView.forceActiveFocus()
    }

    function loadData() {
        if (root.repository !== null)
            root.repository.refreshGraph()
    }
}

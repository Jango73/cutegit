import QtQuick 2.12
import "../components"
import "../views"

CommitLogTabBase {
    id: root

    StandardTextFilter {
        id: filter
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: Const.filterText
        tipText: Const.tipLineFilter

        onFilterTextChanged: {
            if (root.repository !== null)
                root.repository.setGraphFilter(text)
        }
    }

    StandardLabel {
        anchors.fill: graphView
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: Const.listEmptyText
        visible: root.repository === null || graphView.count === 0
    }

    GraphView {
        id: graphView
        anchors.top: filter.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: root.repository ? root.repository.graphModelProxy : undefined
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

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
                root.repository.setBranchLogFilter(text)
        }
    }

    StandardLabel {
        anchors.fill: logView
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: Const.listEmptyText
        visible: root.repository === null || logView.count === 0
    }

    LogView {
        id: logView
        anchors.top: filter.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: root.repository ? root.repository.branchLogModelProxy : undefined
        diffFromCommitId: root.diffFromCommitId
        diffToCommitId: root.diffToCommitId

        onItemClicked: root.showCommitDiffPrevious(commitId)
        onItemRightClicked: root.showCommitMenu(commitId, message)
        onRequestCommitDiffPrevious: root.requestCommitDiffPrevious(commitId)
        onRequestCommitDiffFrom: root.requestCommitDiffFrom(commitId)
        onRequestCommitDiffTo: root.requestCommitDiffTo(commitId)
    }

    function focusTab() {
        logView.forceActiveFocus()
    }

    function loadData() {
        if (root.repository !== null)
            root.repository.refreshBranchLog()
    }
}

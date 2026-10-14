import QtQuick 2.12
import "../components"
import "../views"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

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
        diffFromCommitId: root.repository ? root.repository.diffFromCommitId : ""
        diffToCommitId: root.repository ? root.repository.diffToCommitId : ""

        onItemRightClicked: root.viewRoot.showLogMenu(commitId, message)
        onRequestCommitDiffPrevious: root.viewRoot.requestCommitDiffPrevious(commitId)
        onRequestCommitDiffFrom: root.viewRoot.requestCommitDiffFrom(commitId)
        onRequestCommitDiffTo: root.viewRoot.requestCommitDiffTo(commitId)
    }

    function focusTab() {
        logView.forceActiveFocus()
    }
}

import QtQuick 2.12

BaseTab {
    id: root

    property string diffFromCommitId: root.repository ? root.repository.diffFromCommitId : ""
    property string diffToCommitId: root.repository ? root.repository.diffToCommitId : ""

    function showCommitDiff(commitId) {
        root.viewRoot.showCommitDiffPrevious(commitId)
    }

    function showCommitMenu(commitId, message) {
        root.viewRoot.showLogMenu(commitId, message)
    }

    function requestCommitDiffPrevious(commitId) {
        root.viewRoot.requestCommitDiffPrevious(commitId)
    }

    function requestCommitDiffFrom(commitId) {
        root.viewRoot.requestCommitDiffFrom(commitId)
    }

    function requestCommitDiffTo(commitId) {
        root.viewRoot.requestCommitDiffTo(commitId)
    }
}

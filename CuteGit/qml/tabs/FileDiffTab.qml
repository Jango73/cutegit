import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    FileDiffPane {
        id: diffPane
        anchors.fill: parent
        repository: root.repository
        showCommitDiff: false

        onRequestMenu: function(staged, operation, text, hunkId, selectedRows) {
            root.viewRoot.showDiffMenu(staged, operation, text, hunkId, selectedRows, false)
        }
        onRequestTextFilter: function(text) { if (root.repository !== null) root.repository.setWorkingTreeDiffFilter(text) }
        onRequestSaveAsPatch: root.viewRoot.requestPatchSave(false)
    }

    function focusTab() {
        diffPane.forceActiveFocus()
    }
}

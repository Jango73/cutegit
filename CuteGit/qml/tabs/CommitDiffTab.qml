import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    FileDiffPane {
        id: diffPane
        anchors.fill: parent
        repository: root.repository
        showCommitDiff: true

        onRequestMenu: function(staged, operation, text, hunkId, selectedRows) {
            root.viewRoot.showDiffMenu(staged, operation, text, hunkId, selectedRows, true)
        }
        onRequestTextFilter: if (root.repository !== null) root.repository.setCommitDiffFilter(text)
        onRequestSaveAsPatch: root.viewRoot.requestPatchSave(true)
    }

    function focusTab() {
        diffPane.forceActiveFocus()
    }
}

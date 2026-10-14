import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    FileDiffPane {
        id: diffPane
        anchors.fill: parent
        repository: root.repository

        onRequestMenu: root.viewRoot.showDiffMenu(operation, text)
        onRequestTextFilter: if (root.repository !== null) root.repository.setFileDiffFilter(text)
        onRequestSaveAsPatch: root.viewRoot.requestPatchSave()
    }

    function focusTab() {
        diffPane.forceActiveFocus()
    }
}

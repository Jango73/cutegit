import QtQuick 2.12
import "../pages"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

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

import QtQuick 2.12
import "../pages"

BaseTab {
    id: root

    FilePane {
        id: filePane
        anchors.fill: parent
        extensionParent: root
        repository: root.repository
        flatSelection: root.flatSelection

        onRequestMenu: root.viewRoot.showFileMenu(fileNames)
        onRequestDeleteFiles: root.viewRoot.requestDeleteFiles(fileNames)
        onRequestFileFilter: function(text) { if (root.repository !== null) root.repository.setFileFilter(text) }
        onRequestFileSortField: function(field) { if (root.repository !== null) root.repository.setFileSortField(field) }
        onRequestFileSortDirection: function(direction) { if (root.repository !== null) root.repository.setFileSortDirection(direction) }
    }

    function getSelectedFiles() {
        return filePane.getSelectedFiles()
    }

    function focusTab() {
        filePane.activateFlatFileView()
    }
}

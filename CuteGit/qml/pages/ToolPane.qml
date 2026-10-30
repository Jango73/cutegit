import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "../components"
import "../views"
import "../popups"

ExtendablePane {
    id: root

    property variant repository: null

    signal requestDiffMenu(var operation, var text)
    signal requestSaveFileDiffAsPatch()
    signal requestLogMenu(var commitId, var message)
    signal requestCommitDiffPrevious(var commitId)
    signal requestCommitDiffFrom(var commitId)
    signal requestCommitDiffTo(var commitId)

    content: [
        TabBar {
            id: tabBar
            anchors.top: parent.top

            StandardTabButton {
                width: implicitWidth
                text: Const.diffText
            }

            StandardTabButton {
                width: implicitWidth
                text: Const.fileLogText
            }

            StandardTabButton {
                width: implicitWidth
                text: Const.blameText
            }
        },

        StandardTabView {
            id: swipeView
            anchors.top: tabBar.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Const.smallPadding
            currentIndex: tabBar.currentIndex
            clip: true

            FileDiffPane {
                id: diffView
                repository: root.repository

                onRequestMenu: function(staged, operation, text, hunkId, selectedRows) { root.requestDiffMenu(operation, text) }
                onRequestTextFilter: function(text) { root.repository.setWorkingTreeDiffFilter(text) }
                onRequestSaveAsPatch: root.requestSaveFileDiffAsPatch()
            }

            FileLogPane {
                id: fileLogPane
                repository: root.repository

                onRequestMenu: function(commitId, message) { root.requestLogMenu(commitId, message) }
                onRequestCommitDiffPrevious: function(commitId) { root.requestCommitDiffPrevious(commitId) }
                onRequestCommitDiffFrom: function(commitId) { root.requestCommitDiffFrom(commitId) }
                onRequestCommitDiffTo: function(commitId) { root.requestCommitDiffTo(commitId) }
            }

            BlameView {
                id: blameView
                repository: root.repository

                onRequestTextFilter: function(text) { root.repository.setFileBlameFilter(text) }
            }
        }
    ]

    function activateFileDiffView() {
        tabBar.currentIndex = 0
        diffView.forceActiveFocus()
    }

    function activateFileBlameView() {
        tabBar.currentIndex = 2
        blameView.forceActiveFocus()
    }
}

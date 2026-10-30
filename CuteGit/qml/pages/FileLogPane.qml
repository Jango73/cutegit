import QtQuick 2.12
import QtQuick.Controls 2.5
import "../components"
import "../views"
import "../popups"

Item {
    id: root

    property variant repository: null

    signal requestMenu(var commitId, var message)
    signal requestCommitDiffPrevious(var commitId)
    signal requestCommitDiffFrom(var commitId)
    signal requestCommitDiffTo(var commitId)

    StandardLabel {
        anchors.fill: logView
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: Const.listEmptyText
        visible: root.repository === null | logView.count === 0
    }

    StandardLabel {
        id: logSource
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: root.repository ? root.repository.fileLogModel.relativeSourceName : ""
    }

    LogView {
        id: logView
        anchors.top: logSource.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: root.repository ? root.repository.fileLogModel : undefined
        diffFromCommitId: root.repository ? root.repository.diffFromCommitId : ""
        diffToCommitId: root.repository ? root.repository.diffToCommitId : ""

        onItemRightClicked: function(commitId, message) { root.requestMenu(commitId, message) }
        onRequestCommitDiffPrevious: function(commitId) { root.requestCommitDiffPrevious(commitId) }
        onRequestCommitDiffFrom: function(commitId) { root.requestCommitDiffFrom(commitId) }
        onRequestCommitDiffTo: function(commitId) { root.requestCommitDiffTo(commitId) }
    }
}

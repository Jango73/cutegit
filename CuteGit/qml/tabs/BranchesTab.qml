import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

BaseTab {
    id: root

    Component {
        id: branchDelegate

        StandardListViewItem {
            width: ListView.view.width
            listView: ListView.view
            selectionShown: root.repository !== null && model.name === root.repository.currentBranch
            primaryText: model.name

            onClicked: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    root.showBranchMenuFor(model.name)
                }
            }

            onDoubleClicked: {
                root.switchToBranch(model.name)
            }
        }
    }

    StandardLabel {
        id: remoteBranchesLabel
        width: parent.width
        text: Const.remoteText
        anchors.top: parent.top
        anchors.topMargin: Const.mainPadding
    }

    StandardListView {
        id: remoteBranchList
        anchors.top: remoteBranchesLabel.bottom
        width: parent.width
        height: parent.height * Const.branchListHeightFraction - remoteBranchesLabel.height
        visible: count > 0

        model: root.repository ? root.repository.remoteBranchModel : undefined
        delegate: branchDelegate
    }

    Rectangle {
        id: separator
        width: parent.width
        height: Const.branchSeparatorHeight
        anchors.top: remoteBranchList.bottom
        anchors.topMargin: Const.mainPadding
        color: Material.accent
    }

    StandardLabel {
        id: localBranchesLabel
        width: parent.width
        text: Const.localText
        anchors.top: separator.bottom
        anchors.topMargin: Const.mainPadding
    }

    StandardListView {
        id: localBranchList
        width: parent.width
        anchors.top: localBranchesLabel.bottom
        anchors.bottom: parent.bottom
        visible: count > 0

        model: root.repository ? root.repository.localBranchModel : undefined
        delegate: branchDelegate
    }

    function showBranchMenuFor(branchName) {
        if (root.viewRoot !== null)
            root.viewRoot.showBranchMenu(branchName)
    }

    function switchToBranch(branchName) {
        if (root.repository !== null && root.repository.can(CEnums.SwitchBranch))
            root.repository.currentBranch = branchName
    }

    function focusTab() {
        if (remoteBranchList.count > 0)
            remoteBranchList.forceActiveFocus()
        else
            localBranchList.forceActiveFocus()
    }
}

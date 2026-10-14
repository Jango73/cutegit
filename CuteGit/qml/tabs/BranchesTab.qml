import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

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
        height: parent.height * 0.5 - remoteBranchesLabel.height
        visible: count > 0

        model: root.repository ? root.repository.remoteBranchModel : undefined

        delegate: StandardListViewItem {
            width: ListView.view.width
            listView: remoteBranchList
            selectionShown: root.repository !== null && model.name === root.repository.currentBranch
            primaryText: model.name

            onClicked: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    root.viewRoot.showBranchMenu(model.name)
                }
            }

            onDoubleClicked: {
                if (root.repository.can(CEnums.SwitchBranch)) {
                    root.repository.currentBranch = model.name
                }
            }
        }
    }

    Rectangle {
        id: separator
        width: parent.width
        height: 2
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

        delegate: StandardListViewItem {
            width: ListView.view.width
            listView: localBranchList
            selectionShown: root.repository !== null && model.name === root.repository.currentBranch
            primaryText: model.name

            onClicked: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    root.viewRoot.showBranchMenu(model.name)
                }
            }

            onDoubleClicked: {
                if (root.repository.can(CEnums.SwitchBranch)) {
                    root.repository.currentBranch = model.name
                }
            }
        }
    }

    function focusTab() {
        if (remoteBranchList.count > 0)
            remoteBranchList.forceActiveFocus()
        else
            localBranchList.forceActiveFocus()
    }
}

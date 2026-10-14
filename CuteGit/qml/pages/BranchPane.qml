import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"
import ".."

StandardPane {
    id: root

    property variant repository: null
    property string branchName: ""

    signal requestRebaseOnBranch(var name)
    signal requestMergeBranch(var name)
    signal requestDeleteBranch(var name)

    TabBar {
        id: tabBar
        anchors.top: parent.top

        StandardTabButton {
            width: implicitWidth
            text: Const.branchesText
        }

        StandardTabButton {
            width: implicitWidth
            text: Const.tagsText
        }
    }

    StandardTabView {
        id: swipeView
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Const.smallPadding
        clip: true
        currentIndex: tabBar.currentIndex

        Item {
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
                    selectionShown: model.name === root.repository.currentBranch
                    primaryText: model.name

                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            rightClickBranch(model.name)
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
                    selectionShown: model.name === root.repository.currentBranch
                    primaryText: model.name

                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            rightClickBranch(model.name)
                        }
                    }

                    onDoubleClicked: {
                        if (root.repository.can(CEnums.SwitchBranch)) {
                            root.repository.currentBranch = model.name
                        }
                    }
                }
            }

            BranchMenu {
                id: branchMenu
                repository: root.repository

                onRequestSwitchToBranch: root.repository.currentBranch = root.branchName
                onRequestRebaseOnBranch: root.requestRebaseOnBranch(root.branchName)
                onRequestMergeBranch: root.requestMergeBranch(root.branchName)
                onRequestDeleteBranch: root.requestDeleteBranch(root.branchName)
            }
        }

        Item {
            StandardListView {
                id: tagList
                anchors.fill: parent
                visible: count > 0

                model: root.repository ? root.repository.tagModel : undefined

                delegate: StandardListViewItem {
                    width: ListView.view.width
                    listView: tagList
                    selectionShown: false
                    primaryText: model.name
                }
            }
        }
    }

    function rightClickBranch(name) {
        root.branchName = name

        branchMenu.canRebaseOn = (name !== root.repository.currentBranch)
        branchMenu.canMerge = (name !== root.repository.currentBranch)
        branchMenu.popup()
    }
}

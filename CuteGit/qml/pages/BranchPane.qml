import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "../components"
import ".."

Pane {
    id: root
    padding: Const.panePadding
    anchors.margins: Const.paneMargins

    Material.elevation: Const.paneElevation

    property variant repository: null
    property string branchName: ""

    signal requestMergeBranch(var name)
    signal requestDeleteBranch(var name)

    TabBar {
        id: tabBar
        anchors.top: parent.top

        TabButton {
            width: implicitWidth
            text: Const.branchesText
        }

        TabButton {
            width: implicitWidth
            text: Const.tagsText
        }
    }

    SwipeView {
        id: swipeView
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        interactive: false
        clip: true
        currentIndex: tabBar.currentIndex

        StandardListView {
            id: branchList
            visible: count > 0

            model: root.repository ? root.repository.branchModel : undefined

            delegate: StandardListViewItem {
                width: parent.width
                height: Const.elementHeight + Const.smallPadding
                text: model.name
                selectionShown: model.name === root.repository.currentBranch
                listView: branchList

                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        root.branchName = model.name
                        branchMenu.canMerge = model.name !== root.repository.currentBranch
                        branchMenu.popup()
                    }
                }

                onDoubleClicked: {
                    root.repository.currentBranch = model.name
                }
            }

            BranchMenu {
                id: branchMenu

                onRequestSwitchToBranch: {
                    root.repository.currentBranch = root.branchName
                }

                onRequestMergeBranch: {
                    root.requestMergeBranch(root.branchName)
                }

                onRequestDeleteBranch: {
                    root.requestDeleteBranch(root.branchName)
                }
            }
        }

        StandardListView {
            id: tagList
            visible: count > 0

            model: root.repository.tagModel

            delegate: StandardListViewItem {
                width: parent.width
                height: Const.elementHeight + Const.smallPadding
                text: model.name
                selectionShown: false
                listView: tagList
            }
        }
    }
}

import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "../components"
import "../views"
import "../popups"

ExtendablePane {
    id: root

    /*! The repository for this view */
    property variant repository: null

    signal requestMenu(var commitId, var message)
    signal requestCommitDiffPrevious(var commitId)
    signal requestCommitDiffFrom(var commitId)
    signal requestCommitDiffTo(var commitId)
    signal requestTextFilter(var text)

    TabBar {
        id: tabBar
        anchors.top: parent.top

        TabButton {
            id: branchLogTab
            width: implicitWidth
            text: Const.branchLogText
        }

        TabButton {
            id: graphTab
            width: implicitWidth
            text: Const.graphText
        }

        TabButton {
            id: refLogTab
            width: implicitWidth
            text: Const.refLogText
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
            StandardTextFilter {
                id: filter
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                text: Const.filterText
                tipText: Const.tipLineFilter

                onFilterTextChanged: {
                    root.requestTextFilter(text)
                }
            }

            StandardLabel {
                anchors.fill: logView
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: Const.listEmptyText
                visible: root.repository === null | logView.count === 0
            }

            LogView {
                id: logView
                anchors.top: filter.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                model: root.repository ? root.repository.branchLogModelProxy : undefined
                diffFromCommitId: root.repository ? root.repository.diffFromCommitId : ""
                diffToCommitId: root.repository ? root.repository.diffToCommitId : ""

                onItemRightClicked: root.requestMenu(commitId, message)
                onRequestCommitDiffPrevious: root.requestCommitDiffPrevious(commitId)
                onRequestCommitDiffFrom: root.requestCommitDiffFrom(commitId)
                onRequestCommitDiffTo: root.requestCommitDiffTo(commitId)
            }
        }

        Item {
            GraphView {
                id: graphView
                anchors.fill: parent

                model: root.repository !== null ? root.repository.graphModel : undefined
            }
        }

        Item {
            LogView {
                id: refLogView
                anchors.fill: parent

                model: root.repository !== null ? root.repository.refLogModel : undefined
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Functions

    function activateBranchLogView() {
        tabBar.currentIndex = 0
        logView.forceActiveFocus()
    }

    function activateGraphView() {
        tabBar.currentIndex = 1
        graphView.forceActiveFocus()
    }
}

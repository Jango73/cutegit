import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls 1.4 as QC14
import QtQuick.Controls.Material 2.12
import "../components"

QC14.TabView {
    id: root
    style: StandardTabViewStyle { }

    property variant repository: null
    property string branchName: ""

    signal requestMergeBranch(var name)
    signal requestDeleteBranch(var name)

    QC14.Tab {
        title: Const.branchesText

        Pane {
            anchors.fill: parent
            padding: Const.mainPadding

            StandardListView {
                id: branchList
                anchors.fill: parent
                visible: count > 0

                model: root.repository ? root.repository.branchModel : undefined

                delegate: StandardListViewItem {
                    width: parent.width
                    height: Const.elementHeight + Const.mainPadding * 0.25
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
        }
    }

    QC14.Tab {
        title: Const.tagsText

        Pane {
            anchors.fill: parent
            padding: Const.mainPadding

            StandardListView {
                id: tagList
                anchors.fill: parent
                visible: count > 0

                model: root.repository.tagModel

                delegate: StandardListViewItem {
                    width: parent.width
                    height: Const.elementHeight + Const.mainPadding * 0.25
                    text: model.name
                    selectionShown: false
                    listView: tagList
                }
            }
        }
    }
}

import QtQuick 2.12
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

TreeView {
    id: root
    enabled: visible

    property variant repository: null

    model: root.repository !== null
           ? root.repository.treeFileModelProxy
           : undefined

    rootIndex: root.repository !== null
               ? root.repository.treeFileModelProxy.rootPathIndex
               : undefined

    backgroundVisible: false
    headerVisible: false
    selection: root.selection
    selectionMode: 2
    // "None", "Single", "Extended", "Multi", "Contig."

    TableViewColumn {
        title: Const.nameText
        role: "fileName"
    }

    rowDelegate: rowDlg
    itemDelegate: itemDlg

    Component {
        id: rowDlg

        Rectangle {
            height: Const.treeElementHeight
            color: Material.background
        }
    }

    Component {
        id: itemDlg
        objectName: "itemDlg"

        Rectangle {
            height: Const.treeElementHeight
            color: Material.background

            Rectangle {
                anchors.fill: parent
                color: if (model !== null && typeof model !== "undefined")
                           if (model.staged === Const.staged) Const.fileStagedColor
                           else if (model.status === Const.statusModified) Const.fileModifiedColor
                           else if (model.status === Const.statusRenamed) Const.fileRenamedColor
                           else if (model.status === Const.statusAdded) Const.fileAddedColor
                           else if (model.status === Const.statusDeleted) Const.fileDeletedColor
                           else Const.transparent
                else Const.transparent
            }

            Selection {
                id: selectionIndicator
                anchors.fill: parent
                show: styleData.selected

                FocusIndicator {
                    anchors.fill: parent
                    visible: root.activeFocus && styleData.index === root.currentIndex
                }
            }

            StandardText {
                id: statusText
                width: Const.elementHeight
                height: parent.height
                text: model !== null && typeof model !== "undefined" ? model.status : ""
            }

            TextOverSelection {
                id: fileNameText
                anchors.left: statusText.right
                anchors.right: parent.right
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: model !== null && typeof model !== "undefined" ? model.fileName : ""

                selection: selectionIndicator
            }
        }
    }

    onDoubleClicked: {
        var currentFileFullName = model.fullNameForIndex(currentIndex)
        root.repository.openFile(currentFileFullName)
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Space) {
            var currentFileFullName = model.fullNameForIndex(currentIndex)
            root.repository.toggleStaged(currentFileFullName)
        }
    }

    /*
    TODO: make those work
    function expandAll() {
        for (var i = 0; i < root.model.rowCount(); i++) {
            var index = root.model.index(i, 0)
            console.log(index)

            if (!treeView.isExpanded(index)) {
                root.expand(index)
            }
        }
    }

    function collapseAll() {
    }
    */
}

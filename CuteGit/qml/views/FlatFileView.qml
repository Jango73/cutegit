import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQml.Models
import CuteGit 1.0
import "../components"

StandardListView {
    id: root

    property variant repository: null
    property bool mouseEnabled: true

    signal requestMenu(var fileNames)
    signal requestDeleteFiles(var fileNames)

    model: root.repository
           ? root.repository.flatFileModelProxy
           : undefined

    delegate: StandardListViewItem {
        objectName: "FlatFileView.delegate." + index
        width: parent.width
        expanded: mustShowRelativeName
        selectionFillsItem: false
        listView: root
        mouseEnabled: root.mouseEnabled
        symbolText: model.changing ? Const.statusChangeText : model.status
        primaryText: model.fileName
        secondaryText: mustShowRelativeName ? model.relativeName : ""
        selectionShown: selected
        focusShown: root.activeFocus && index === root.currentIndex

        property string fullName: model.fullName
        property bool mustShowRelativeName: model.fileName !== model.relativeName
        property bool selected: indexSelected(index)

        onClicked: {
            root.itemClicked(mouse, index, previousIndex)

            if (mouse.button === Qt.RightButton) {
                root.requestMenu(getSelectedFiles())
            }
        }

        onDoubleClicked: root.repository.openFiles(getSelectedFiles())

        onFullNameChanged: root.updateModelIndex(index)

        Component.onCompleted: root.updateModelIndex(index)

        background: Rectangle {
            anchors.fill: parent
            color: if (model.staged === Const.staged) Const.fileStagedColor
                   else if (model.status === Const.statusModified) Const.fileModifiedColor
                   else if (model.status === Const.statusRenamed) Const.fileRenamedColor
                   else if (model.status === Const.statusAdded) Const.fileAddedColor
                   else if (model.status === Const.statusDeleted) Const.fileDeletedColor
                   else Const.transparent
        }
    }

    onSpacePressed: root.repository.toggleStaged(getSelectedFiles())

    onDeletePressed: root.requestDeleteFiles(getSelectedFiles())

    function getSelectedFiles() {
        return root.model.selectionToFullNameList(root.selection.selectedIndexes)
    }
}

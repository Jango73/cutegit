import QtQuick 2.12
import QtQml 2.12
import QtQml.Models 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

StandardListView {
    id: root

    signal itemRightClicked(var operation, var text, var hunkId)

    /*! Repository used to stage lines with Space, null disables staging */
    property variant repository: null

    /*! True for the staged pane (Space unstages), false for the unstaged pane */
    property bool staged: false

    /*! If false, Space never stages (non-indexed version controls, commit view) */
    property bool stagingEnabled: true

    /*! Selection owned by the view, bound to the proxy model */
    selection: ItemSelectionModel {
        model: root.model
    }

    /*! Selected proxy rows, read by the diff menu and staging helpers */
    readonly property var selectedRows: root.selection
        ? root.selection.selectedIndexes
            .map(function(index) { return index.row })
            .sort(function(first, second) { return first - second })
        : []

    function stageSelection() {
        if (!stagingEnabled || !root.repository)
            return

        var rows = selectedRows.length !== 0
            ? selectedRows.slice()
            : (currentIndex > Const.invalidRow ? [currentIndex] : [])

        if (rows.length === 0)
            return

        if (root.staged)
            root.repository.unstageDiffLines(rows)
        else
            root.repository.stageDiffLines(rows)
    }

    /*! Toggles a single row without clearing the selection (Control+click) */
    function toggleSelection(row) {
        if (!selection)
            return

        root.updateModelIndex(row)

        var proxyIndex = modelIndices[row]

        if (proxyIndex === undefined)
            return

        selection.select(proxyIndex, ItemSelectionModel.Toggle)
        currentIndex = row
    }

    function headerText(isCollapsed, text) {
        return (isCollapsed ? Const.collapsedHeaderPrefix : Const.expandedHeaderPrefix) + text
    }

    Connections {
        target: root.model ? root.model : null
        function onModelReset() {
            if (root.selection)
                root.selection.clear()
        }
    }

    onSpacePressed: root.stageSelection()

    delegate: StandardListViewItem {
        objectName: "root.delegate." + index
        width: ListView.view.width
        listView: root
        primaryText: model.operation === CEnums.DiffFileName
            ? root.headerText(model.collapsed, model.text)
            : model.text
        primaryColor: model.operation === CEnums.DiffFileName ? Material.background : Material.foreground
        primaryFont.family: Const.fixedWidthFontFamily
        primaryFont.pixelSize: Const.fixedWidthFontSize
        selectionShown: root.indexSelected(index)

        /*! Tracks delegate reuse across rows: index has no notifier of its own */
        property int rowIndex: index
        onRowIndexChanged: root.updateModelIndex(rowIndex)
        Component.onCompleted: root.updateModelIndex(index)

        background: Rectangle {
            anchors.fill: parent
            color: if (model.operation === CEnums.DiffAdd) Const.fileStagedColor
                   else if (model.operation === CEnums.DiffDelete) Const.fileModifiedColor
                   else if (model.operation === CEnums.DiffFileName) Material.accent
                   else if (model.operation === CEnums.DiffHunk) Const.hunkHeaderColor
                   else Const.transparent
        }

        onClicked: function(mouse, previousIndex) {
            if (mouse.button === Qt.LeftButton) {
                if (model.operation === CEnums.DiffFileName) {
                    ListView.view.model.toggleCollapsedFile(model.text)
                }

                if (mouse.modifiers & Qt.ControlModifier) {
                    ListView.view.toggleSelection(index)
                } else {
                    ListView.view.itemClicked(mouse, index, previousIndex)
                }
            } else if (mouse.button === Qt.RightButton) {
                ListView.view.itemClicked(mouse, index, previousIndex)
                ListView.view.itemRightClicked(model.operation, model.text, model.hunkId)
            }
        }
    }
}

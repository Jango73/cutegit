import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

StandardListView {
    id: root

    signal itemRightClicked(var proxyRow, var operation, var text, var hunkId)

    /*! Repository used to stage lines with Space, null disables staging */
    property variant repository: null

    /*! True for the staged pane (Space unstages), false for the unstaged pane */
    property bool staged: false

    /*! If false, Space never stages (non-indexed version controls, commit view) */
    property bool stagingEnabled: true

    /*! Selected proxy rows */
    property var selectedRows: []

    /*! Anchor row for Shift range extension */
    property int selectionAnchor: -1

    function isSelected(row) {
        return selectedRows.indexOf(row) !== -1
    }

    function clearSelection() {
        if (selectedRows.length !== 0)
            selectedRows = []

        selectionAnchor = -1
    }

    function selectOnly(row) {
        selectedRows = [row]
        selectionAnchor = row
        currentIndex = row
    }

    function toggleRow(row) {
        var rows = selectedRows.slice()
        var position = rows.indexOf(row)

        if (position === -1)
            rows.push(row)
        else
            rows.splice(position, 1)

        selectedRows = rows
        selectionAnchor = row
        currentIndex = row
    }

    function selectRangeTo(row, fallbackAnchor) {
        var anchor = selectionAnchor

        if (anchor < 0)
            anchor = (fallbackAnchor === undefined ? -1 : fallbackAnchor)

        if (anchor < 0)
            anchor = row

        var from = Math.min(anchor, row)
        var to = Math.max(anchor, row)
        var rows = []

        for (var index = from; index <= to; index++)
            rows.push(index)

        selectedRows = rows
        currentIndex = row
    }

    function selectAllRows() {
        var rows = []

        for (var index = 0; index < count; index++)
            rows.push(index)

        selectedRows = rows
    }

    function stageSelection() {
        if (!stagingEnabled || !root.repository)
            return

        var rows = selectedRows.length !== 0
            ? selectedRows.slice()
            : (currentIndex >= 0 ? [currentIndex] : [])

        if (rows.length === 0)
            return

        if (root.staged)
            root.repository.unstageDiffLines(rows)
        else
            root.repository.stageDiffLines(rows)
    }

    /*! Extends the line selection on Shift, moves it otherwise */
    function handleKeyboardMovement(event, targetIndex) {
        var clamped = Math.max(0, Math.min(count - 1, targetIndex))

        if (event.modifiers & Qt.ShiftModifier)
            selectRangeTo(clamped, currentIndex)
        else
            selectOnly(clamped)
    }

    function headerText(isCollapsed, text) {
        return (isCollapsed ? "▶ " : "▼ ") + text
    }

    Connections {
        target: root.model
        function onModelReset() { root.clearSelection() }
    }

    onSpacePressed: root.stageSelection()

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_A && (event.modifiers & Qt.ControlModifier)) {
            root.selectAllRows()
            event.accepted = true
        }
    }

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
        selectionShown: root.selectedRows.indexOf(index) !== -1

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
                    root.toggleRow(index)
                } else if (mouse.modifiers & Qt.ShiftModifier) {
                    root.selectRangeTo(index, previousIndex)
                } else {
                    root.selectOnly(index)
                }
            } else if (mouse.button === Qt.RightButton) {
                if (root.selectedRows.indexOf(index) === -1) {
                    root.selectOnly(index)
                }

                root.itemRightClicked(index, model.operation, model.text, model.hunkId)
            }
        }
    }
}

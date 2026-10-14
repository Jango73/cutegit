import QtQuick 2.12
import QtQuick.Controls 2.5
import "../components"

Action {
    id: root

    /*! Menu owning the selection state and staging helpers, null when detached */
    property var menu: null

    /*! True when the action targets the staged pane */
    property bool forStaged: false

    /*! True for whole-hunk actions, false for selected-lines actions */
    property bool forHunk: false

    text: {
        if (root.forHunk)
            return root.forStaged ? Const.unstageHunkText : Const.stageHunkText

        return root.forStaged ? Const.unstageSelectedLinesText : Const.stageSelectedLinesText
    }

    enabled: root.menu !== null && root.menu.isActionEnabled(root.forStaged, root.forHunk)

    onTriggered: {
        if (root.menu !== null)
            root.menu.triggerAction(root.forStaged, root.forHunk)
    }
}

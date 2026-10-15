import QtQuick 2.12
import QtQuick.Controls 2.5
import CuteGit 1.0
import "../components"

Menu {
    id: root
    title: ""

    /*! The repository for this view */
    property variant repository: null

    /*! True when the menu was opened from the staged pane */
    property bool staged: false

    /*! True when the menu was opened from the commit diff pane (no staging there) */
    property bool fromCommit: false

    /*! The currently selected operation */
    property string operation: ""

    /*! The currently selected text */
    property string text: ""

    /*! The hunk owning the clicked row, invalid when none */
    property int hunkId: Const.invalidHunkId

    /*! Selected proxy rows at menu time */
    property var selectedRows: []

    /*! True when line or hunk staging actions apply to the attached repository */
    property bool isStageable: root.repository !== null
        && root.fromCommit === false
        && Const.supportsStaging(root.repository, CEnums.Git, CEnums.Gerrit)

    /*! Requests a copy action */
    signal requestCopyText(var text)

    /*! Shared enabled rule for line and hunk staging actions */
    function isActionEnabled(forStaged, forHunk) {
        if (!root.isStageable || root.staged !== forStaged)
            return false

        if (forHunk)
            return root.hunkId !== Const.invalidHunkId

        return root.selectedRows.length > 0
    }

    /*! Shared trigger for line and hunk staging actions */
    function triggerAction(forStaged, forHunk) {
        if (root.repository === null)
            return

        if (forHunk) {
            if (forStaged)
                root.repository.unstageDiffHunk(root.hunkId)
            else
                root.repository.stageDiffHunk(root.hunkId)
        } else {
            if (forStaged)
                root.repository.unstageDiffLines(root.selectedRows)
            else
                root.repository.stageDiffLines(root.selectedRows)
        }
    }

    Action {
        text: Const.diffCopyLineText

        onTriggered: {
            if (root.text !== "") {
                root.requestCopyText(root.text)
            }
        }
    }

    DiffMenuAction {
        menu: root
        forStaged: false
        forHunk: false
    }

    DiffMenuAction {
        menu: root
        forStaged: true
        forHunk: false
    }

    DiffMenuAction {
        menu: root
        forStaged: false
        forHunk: true
    }

    DiffMenuAction {
        menu: root
        forStaged: true
        forHunk: true
    }
}

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

    /*! The clicked proxy row */
    property int proxyRow: -1

    /*! The currently selected operation */
    property string operation: ""

    /*! The currently selected text */
    property string text: ""

    /*! The hunk owning the clicked row, -1 when none */
    property int hunkId: -1

    /*! Selected proxy rows at menu time */
    property var selectedRows: []

    /*! True for Git-like repositories holding an index */
    property bool isStagable: repository !== null
        && (repository.repositoryType === CEnums.Git || repository.repositoryType === CEnums.Gerrit)
        && repository.fileDiffFromCommit === false

    /*! Requests a copy action */
    signal requestCopyText(var text)

    Action {
        text: Const.diffCopyLineText

        onTriggered: {
            if (root.text !== "") {
                root.requestCopyText(root.text)
            }
        }
    }

    Action {
        text: Const.stageSelectedLinesText
        enabled: root.isStagable && root.staged === false && root.selectedRows.length > 0

        onTriggered: {
            if (root.repository !== null) {
                root.repository.stageDiffLines(root.selectedRows)
            }
        }
    }

    Action {
        text: Const.unstageSelectedLinesText
        enabled: root.isStagable && root.staged === true && root.selectedRows.length > 0

        onTriggered: {
            if (root.repository !== null) {
                root.repository.unstageDiffLines(root.selectedRows)
            }
        }
    }

    Action {
        text: Const.stageHunkText
        enabled: root.isStagable && root.staged === false && root.hunkId >= 0

        onTriggered: {
            if (root.repository !== null) {
                root.repository.stageDiffHunk(root.hunkId)
            }
        }
    }

    Action {
        text: Const.unstageHunkText
        enabled: root.isStagable && root.staged === true && root.hunkId >= 0

        onTriggered: {
            if (root.repository !== null) {
                root.repository.unstageDiffHunk(root.hunkId)
            }
        }
    }
}

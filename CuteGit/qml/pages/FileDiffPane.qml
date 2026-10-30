import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"
import "../views"
import "../popups"

Item {
    id: root

    property variant repository: null

    /*! True to show the requested commit diff, false for the working tree diff */
    property bool showCommitDiff: false

    /*! True when the attached repository holds a staging index */
    property bool stagingSupported: Const.supportsStaging(root.repository, CEnums.Git, CEnums.Gerrit)

    /*! Commit diffs have no staged notion, staging applies to the working tree pane only */
    property bool fromCommit: root.showCommitDiff

    /*! Requests to show the menu */
    signal requestMenu(var staged, var operation, var text, var hunkId, var selectedRows)

    /*! Requests a filter change */
    signal requestTextFilter(var text)

    /*! Requests to save the diff as a patch */
    signal requestSaveAsPatch()

    /*! Forwards a working tree right click with its staged side */
    function requestWorkingTreeMenu(staged, operation, text, hunkId, selectedRows) {
        root.requestMenu(staged, operation, text, hunkId, selectedRows)
    }

    /*! True when at least one visible diff file is expanded */
    property bool hasExpandedFiles: {
        if (root.fromCommit)
            return commitDiffView.model ? commitDiffView.model.hasExpandedFiles : false

        var unstagedExpanded = unstagedDiffView.model ? unstagedDiffView.model.hasExpandedFiles : false
        var stagedExpanded = stagedDiffView.model ? stagedDiffView.model.hasExpandedFiles : false
        return unstagedExpanded || stagedExpanded
    }

    /*! Collapses every file in all visible diff views */
    function collapseAll() {
        if (commitDiffView.model)
            commitDiffView.model.collapseAll()
        if (unstagedDiffView.model)
            unstagedDiffView.model.collapseAll()
        if (stagedDiffView.model)
            stagedDiffView.model.collapseAll()
    }

    /*! Expands every file in all visible diff views */
    function expandAll() {
        if (commitDiffView.model)
            commitDiffView.model.expandAll()
        if (unstagedDiffView.model)
            unstagedDiffView.model.expandAll()
        if (stagedDiffView.model)
            stagedDiffView.model.expandAll()
    }

    StandardLabel {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: Const.listEmptyText
        visible: root.repository === null || (
            root.fromCommit ? commitDiffView.count === 0
            : (root.stagingSupported ? (stagedDiffView.count === 0 && unstagedDiffView.count === 0)
                          : unstagedDiffView.count === 0)
        )
    }

    StandardLabel {
        id: diffSource
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: {
            if (root.repository === null)
                return ""

            if (root.fromCommit)
                return Const.commitDiffText + ": " + root.repository.fileDiffModel.relativeSourceName

            var unstagedName = root.repository.unstagedFileDiffModel.relativeSourceName
            if (unstagedName !== "")
                return unstagedName

            return root.repository.stagedFileDiffModel.relativeSourceName
        }
    }

    RowLayout {
        id: filterLayout
        anchors.top: diffSource.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: Const.mainPadding

        StandardTextFilter {
            id: filter
            Layout.fillWidth: true
            text: Const.filterText
            tipText: Const.tipLineFilter

            onFilterTextChanged: function(text) { root.requestTextFilter(text) }
        }

        StandardButton {
            id: saveAsPatch
            text: Const.saveAsPatchText
            enabled: root.fromCommit ? !commitDiffView.empty : (!stagedDiffView.empty || !unstagedDiffView.empty)

            onClicked: {
                root.requestSaveAsPatch()
            }
        }

        StandardButton {
            id: collapseAllButton
            text: Const.collapseAllText
            visible: root.hasExpandedFiles

            onClicked: root.collapseAll()
        }

        StandardButton {
            id: expandAllButton
            text: Const.expandAllText
            visible: !root.hasExpandedFiles

            onClicked: root.expandAll()
        }
    }

    DiffView {
        id: commitDiffView
        objectName: "commitDiffView"
        anchors.top: filterLayout.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: root.fromCommit
        stagingEnabled: false

        model: root.repository !== null ? root.repository.fileDiffModelProxy : null

        onItemRightClicked: function(operation, text, hunkId) {
            root.requestMenu(false, operation, text, hunkId, commitDiffView.selectedRows)
        }
    }

    ColumnLayout {
        anchors.top: filterLayout.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: Const.smallPadding
        visible: !root.fromCommit

        StandardLabel {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft
            text: Const.diffSectionCountText.arg(Const.unstagedDiffText).arg(unstagedDiffView.count)
        }

        WorkingTreeDiffView {
            id: unstagedDiffView
            objectName: "unstagedDiffView"
            stretchWeight: Const.unstagedDiffStretch
            repository: root.repository
            staged: false
            stagingEnabled: root.stagingSupported

            model: root.repository !== null ? root.repository.unstagedFileDiffModelProxy : null

            onRequestMenu: function(operation, text, hunkId, selectedRows) {
                root.requestWorkingTreeMenu(false, operation, text, hunkId, selectedRows)
            }
        }

        StandardLabel {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: Const.noUnstagedChangesText
            visible: unstagedDiffView.count === 0
        }

        StandardLabel {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft
            text: Const.diffSectionCountText.arg(Const.stagedDiffText).arg(stagedDiffView.count)
            visible: root.stagingSupported
        }

        WorkingTreeDiffView {
            id: stagedDiffView
            objectName: "stagedDiffView"
            stretchWeight: Const.stagedDiffStretch
            visible: root.stagingSupported
            repository: root.repository
            staged: true
            stagingEnabled: root.stagingSupported

            model: root.repository !== null ? root.repository.stagedFileDiffModelProxy : null

            onRequestMenu: function(operation, text, hunkId, selectedRows) {
                root.requestWorkingTreeMenu(true, operation, text, hunkId, selectedRows)
            }
        }

        StandardLabel {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: Const.noStagedChangesText
            visible: root.stagingSupported && stagedDiffView.count === 0
        }
    }
}

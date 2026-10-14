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

    /*! True for Git-like repositories holding an index */
    property bool isGit: repository !== null
        && (repository.repositoryType === CEnums.Git || repository.repositoryType === CEnums.Gerrit)

    /*! True when a commit diff is shown instead of the working tree */
    property bool fromCommit: repository !== null && repository.fileDiffFromCommit === true

    /*! Requests to show the menu */
    signal requestMenu(var staged, var proxyRow, var operation, var text, var hunkId, var selectedRows)

    /*! Requests a filter change */
    signal requestTextFilter(var text)

    /*! Requests to save the diff as a patch */
    signal requestSaveAsPatch()

    StandardLabel {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: Const.listEmptyText
        visible: root.repository === null || (
            root.fromCommit ? commitDiffView.count === 0
            : (root.isGit ? (stagedDiffView.count === 0 && unstagedDiffView.count === 0)
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
                return root.repository.fileDiffModel.relativeSourceName

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

            onFilterTextChanged: root.requestTextFilter(text)
        }

        StandardButton {
            id: saveAsPatch
            text: Const.saveAsPatchText
            enabled: root.fromCommit ? !commitDiffView.empty : (!stagedDiffView.empty || !unstagedDiffView.empty)

            onClicked: {
                root.requestSaveAsPatch()
            }
        }
    }

    DiffView {
        id: commitDiffView
        anchors.top: filterLayout.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: root.fromCommit
        stagingEnabled: false

        model: root.repository !== null ? root.repository.fileDiffModelProxy : undefined

        onItemRightClicked: function(proxyRow, operation, text, hunkId) {
            root.requestMenu(false, proxyRow, operation, text, hunkId, commitDiffView.selectedRows)
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
            text: Const.unstagedDiffText + " (" + unstagedDiffView.count + ")"
        }

        DiffView {
            id: unstagedDiffView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: root.isGit ? 2 : 1
            repository: root.repository
            staged: false
            stagingEnabled: root.isGit

            model: root.repository !== null ? root.repository.unstagedFileDiffModelProxy : undefined

            onItemRightClicked: function(proxyRow, operation, text, hunkId) {
                root.requestMenu(false, proxyRow, operation, text, hunkId, unstagedDiffView.selectedRows)
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
            text: Const.stagedDiffText + " (" + stagedDiffView.count + ")"
            visible: root.isGit
        }

        DiffView {
            id: stagedDiffView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1
            visible: root.isGit
            repository: root.repository
            staged: true
            stagingEnabled: root.isGit

            model: root.repository !== null ? root.repository.stagedFileDiffModelProxy : undefined

            onItemRightClicked: function(proxyRow, operation, text, hunkId) {
                root.requestMenu(true, proxyRow, operation, text, hunkId, stagedDiffView.selectedRows)
            }
        }

        StandardLabel {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: Const.noStagedChangesText
            visible: root.isGit && stagedDiffView.count === 0
        }
    }
}

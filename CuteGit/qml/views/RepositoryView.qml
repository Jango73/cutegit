import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs
import QtQml.Models
import CuteGit 1.0
import "../components"
import "../pages"
import "../popups"

Pane {
    id: root
    padding: Const.panePadding

    property variant controller: null
    property variant repository: null
    property string menuBranchName: ""
    property var activeTabIds: []

    //--------------------------------------------------------------------------------
    // Status

    StandardPane {
        id: repositoryStatus
        objectName: "repositoryStatus"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: Const.repositoryStatusHeight

        Item {
            id: repositoryStatusTop
            objectName: "repositoryStatusTop"
            width: parent.width
            height: parent.height * Const.halfFraction

            Rectangle {
                anchors.fill: parent

                color: if (root.repository) {
                           root.repository.repositoryStatus === CEnums.NoMerge
                                   ? Const.transparent
                                   : Material.accent
                       }
                       else Const.transparent
            }

            StandardText {
                id: repositoryTypeText
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                verticalAlignment: Text.AlignVCenter

                color: root.repository
                       ? root.repository.repositoryStatus === CEnums.NoMerge
                         ? Material.foreground
                         : Material.background
                       : "black"

                text: root.repository ? root.repository.repositoryTypeString + " :" : ""
            }

            LogLabel {
                id: trackingStateLabel
                anchors.left: repositoryTypeText.right
                anchors.leftMargin: Const.mainPadding
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: root.repository !== null

                text: root.repository
                      ? "%1 %2 / %3 %4"
                        .arg(Const.aheadText)
                        .arg(root.repository.commitCountAhead)
                        .arg(Const.behindText)
                        .arg(root.repository.commitCountBehind)
                      : ""

                type: root.repository
                      ? root.repository.commitCountAhead === 0 && root.repository.commitCountBehind === 0
                        ? CEnums.UpToDateLabel
                        : root.repository.commitCountAhead > 0 && root.repository.commitCountBehind > 0
                          ? CEnums.DivergedLabel
                          : CEnums.UnsyncedLabel
                      : CEnums.UnknownLabel
            }

            StandardText {
                anchors.left: trackingStateLabel.right
                anchors.leftMargin: Const.mainPadding
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                verticalAlignment: Text.AlignVCenter

                color: root.repository
                       ? root.repository.repositoryStatus === CEnums.NoMerge
                         ? Material.foreground
                         : Material.background
                       : "black"

                text: if (root.repository) {
                          if (root.repository.repositoryStatus === CEnums.InteractiveRebase)
                              ": " + Const.interactiveRebaseProgressText
                          else if (root.repository.repositoryStatus === CEnums.Rebase)
                              ": " + Const.rebaseProgressText
                          else if (root.repository.repositoryStatus === CEnums.Merge)
                              ": " + Const.mergeProgressText
                          else ":"
                      } else ""
            }
        }

        StandardText {
            id: repositoryStatusBottom
            objectName: "repositoryStatusBottom"
            width: parent.width
            anchors.top: repositoryStatusTop.bottom
            anchors.bottom: parent.bottom
            verticalAlignment: Text.AlignVCenter
            text: root.repository
                  ? "%1 : %2".arg(root.repository.repositoryPath).arg(root.repository.currentBranch)
                  : ""
        }
    }

    //--------------------------------------------------------------------------------
    // File selection shared by every Files tab of every panel

    ItemSelectionModel {
        id: flatFileSelection
        objectName: "flatFileSelection"
        model: root.repository ? root.repository.flatFileModelProxy : null

        onCurrentIndexChanged: {
            if (root.repository !== null)
                root.repository.flatFileModelProxy.handleCurrentIndex(currentIndex)
        }
    }

    //--------------------------------------------------------------------------------
    // Dynamic panels driven by the descriptive layout

    Item {
        id: contentArea
        objectName: "contentArea"
        anchors.top: repositoryStatus.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Repeater {
            id: panelRepeater
            objectName: "panelRepeater"
            model: root.controller ? root.controller.currentLayoutPanels : []

            onItemAdded: root.refreshActiveTabs()
            onItemRemoved: root.refreshActiveTabs()

            DockPanel {
                objectName: "dockPanel." + index
                x: root.panelGeometry(modelData, Const.panelPosXKey, 0.0) / Const.percentDivisor * contentArea.width + Const.paneMargins * Const.halfFraction
                y: root.panelGeometry(modelData, Const.panelPosYKey, 0.0) / Const.percentDivisor * contentArea.height + Const.paneMargins * Const.halfFraction
                width: root.panelGeometry(modelData, Const.panelWidthKey, 100.0) / Const.percentDivisor * contentArea.width - Const.paneMargins
                height: root.panelGeometry(modelData, Const.panelHeightKey, 100.0) / Const.percentDivisor * contentArea.height - Const.paneMargins

                extensionParent: root
                repository: root.repository
                flatSelection: flatFileSelection
                viewRoot: root
                panelDefinition: modelData
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Menus

    BranchMenu {
        id: branchMenu
        objectName: "branchMenu"
        repository: root.repository

        onRequestSwitchToBranch: {
            if (root.repository !== null && root.repository.can(CEnums.SwitchBranch))
                root.repository.currentBranch = root.menuBranchName
        }
        onRequestRebaseOnBranch: root.requestRebaseOnBranch(root.menuBranchName)
        onRequestMergeBranch: root.requestMergeBranch(root.menuBranchName)
        onRequestDeleteBranch: root.requestDeleteBranch(root.menuBranchName)
    }

    Action {
        id: rebaseOnBranchAction
        property string targetBranchName: ""
        onTriggered: root.repository.rebaseOnBranch(targetBranchName)
    }

    Action {
        id: mergeBranchAction
        property string targetBranchName: ""
        onTriggered: root.repository.mergeBranch(targetBranchName)
    }

    Action {
        id: deleteBranchAction
        property string targetBranchName: ""
        onTriggered: root.repository.deleteBranch(targetBranchName)
    }

    //--------------------------------------------------------------------------------
    // Menus

    DiffMenu {
        id: diffMenu
        objectName: "diffMenu"
        repository: root.repository

        onRequestCopyText: function(text) {
            root.repository.copyText(text)
        }
    }

    LogMenu {
        id: logMenu
        objectName: "logMenu"
        repository: root.repository

        onRequestCopyText: function(text) {
            root.repository.copyText(text)
        }

        onRequestCommitTagOn: function(commitId) {
            tagOn.commitId = commitId
            tagOn.open()
        }

        onRequestCommitBranchFrom: function(commitId) {
            branchFrom.commitId = commitId
            branchFrom.open()
        }

        onRequestCommitReset: function(commitId) {
            root.repository.commitReset(commitId)
        }

        onRequestCommitRebase: function(commitId) {
            root.repository.commitRebase(commitId)
        }

        onRequestCommitSquash: function(commitId) {
            root.repository.commitSquash(commitId)
        }

        onRequestCommitMessageChange: function(commitId, commitMessage) {
            commit.messageEnabled = true
            commit.messageText = commitMessage
            commit.showFileList = false
            commit.amend = false
            commit.commitId = commitId
            commit.open()
        }

        onRequestCommitDiffPrevious: function(commitId) {
            root.requestCommitDiffPrevious(commitId)
        }

        onRequestCommitDiffFrom: function(commitId) {
            root.requestCommitDiffFrom(commitId)
        }

        onRequestCommitDiffTo: function(commitId) {
            root.requestCommitDiffTo(commitId)
        }
    }

    FileMenu {
        id: fileMenu
        objectName: "fileMenu"
        repository: root.repository

        onRequestDelete: function(fileNames) { root.requestDeleteFiles(fileNames) }
        onRequestBlame: function(fileNames) { root.requestBlameFiles(fileNames) }
        onRequestOpenSubmodule: function(fileNames) { root.requestOpenSubmodule(fileNames) }
    }

    //--------------------------------------------------------------------------------
    // Popups

    ConfirmPopup {
        id: confirm
        objectName: "confirm"
        anchors.centerIn: parent
    }

    CommitPopup {
        id: commit
        objectName: "commit"
        anchors.centerIn: parent

        repository: root.repository
    }

    TagOnPopup {
        id: tagOn
        objectName: "tagOn"
        anchors.centerIn: parent

        repository: root.repository
    }

    BranchFromPopup {
        id: branchFrom
        objectName: "branchFrom"
        anchors.centerIn: parent

        repository: root.repository
    }

    FileDialog {
        id: fileDialog
        objectName: "fileDialog"

        property Action actionOnAccept: null
        property Action actionOnReject: null

        onAccepted: {
            if (actionOnAccept)
            {
                actionOnAccept.fileName = selectedFile
                actionOnAccept.trigger()
            }
        }

        onRejected: {
            if (actionOnReject !== null)
                actionOnReject.trigger()
        }
    }

    Action {
        id: deleteFileAction
        property variant fileNames: ""
        onTriggered: root.repository.deleteFiles(fileNames)
    }

    Action {
        id: savePatchAction
        property string fileName: ""
        property bool forCommit: false
        onTriggered: {
            if (forCommit)
                root.repository.saveCommitDiffAsPatch(fileName)
            else
                root.repository.saveWorkingTreeDiffAsPatch(fileName)
        }
    }

    Action {
        id: applyPatchAction
        property string fileName: ""
        onTriggered: root.repository.patchApply(fileName)
    }

    //--------------------------------------------------------------------------------
    // Functions

    function stringListToString(list) {
        var result = ""
        for (var index = 0; index < list.length; index++) {
            result = result + list[index] + "\n"
        }
        console.log(result)
        return result
    }

    function requestDeleteFiles(fileNames) {
        deleteFileAction.fileNames = fileNames
        confirm.title = Const.deleteFileText
        confirm.messageText = Const.deleteFileMessage.arg(stringListToString(fileNames))
        confirm.actionOnAccept = deleteFileAction
        confirm.actionOnReject = null
        confirm.open()
    }

    function requestBlameFiles(fileNames) {
        if (fileNames.length > 0) {
            root.repository.blame(fileNames[0])
            root.activateFileBlameView()
        }
    }

    function requestOpenSubmodule(fileNames) {
        if (fileNames.length > 0) {
            root.controller.openRepository(fileNames[0])
        }
    }

    function requestRefresh() {
        root.repository.refresh()
    }

    function requestFetch() {
        root.repository.fetch()
    }

    function requestPull() {
        root.repository.pull()
    }

    function requestPush() {
        root.repository.push()
    }

    function requestForcePush() {
        root.repository.forcePush()
    }

    function requestPushAsWIP() {
        root.repository.pushAsWIP()
    }

    function requestCleanUp() {
        root.repository.cleanUp()
    }

    function requestUpdateSubmodules() {
        root.repository.updateSubmodules()
    }

    function requestStageAll() {
        root.repository.stageAll()
    }

    function requestUnstageAll() {
        root.repository.unstageAll()
    }

    function requestStageSelection() {
        root.repository.stageSelection(root.getSelectedFiles())
    }

    function requestUnstageSelection() {
        root.repository.unstageSelection(root.getSelectedFiles())
    }

    function requestRevertSelection() {
        root.repository.revertSelection(root.getSelectedFiles())
    }

    function requestStashSave() {
        root.repository.stashSave()
    }

    function requestStashPop() {
        root.repository.stashPop()
    }

    function requestCommit() {
        commit.messageEnabled = true
        commit.messageText = ""
        commit.showFileList = true
        commit.amend = false
        commit.open()
    }

    function requestAmend() {
        commit.messageEnabled = false
        commit.messageText = Const.amendingText
        commit.showFileList = true
        commit.amend = true
        commit.open()
    }

    function requestContinueRebase() {
        root.repository.continueRebase()
    }

    function requestAbortRebase() {
        root.repository.abortRebase()
    }

    function requestCommitDiffPrevious(commitId) {
        root.repository.commitDiffPrevious(commitId)
        root.activateCommitDiffView()
    }

    function showCommitDiffPrevious(commitId) {
        root.repository.commitDiffPrevious(commitId)
    }

    function requestCommitDiffFrom(commitId) {
        root.repository.diffFromCommitId = commitId
    }

    function requestCommitDiffTo(commitId) {
        root.repository.diffToCommitId = commitId
        root.activateCommitDiffView()
    }

    function requestPatchSave(forCommit) {
        savePatchAction.forCommit = forCommit === true
        fileDialog.title = Const.selectPatchToSaveText
        fileDialog.fileMode = FileDialog.SaveFile
        fileDialog.actionOnAccept = savePatchAction
        fileDialog.nameFilters = [ "Patch files (*.patch)", "All files (*)" ]
        fileDialog.open()
    }

    function requestPatchApply() {
        fileDialog.title = Const.selectPatchToApplyText
        fileDialog.fileMode = FileDialog.OpenFile
        fileDialog.actionOnAccept = applyPatchAction
        fileDialog.nameFilters = ["All files (*)" ]
        fileDialog.open()
    }

    function showFileMenu(fileNames) {
        fileMenu.fileNames = fileNames
        fileMenu.popup()
    }

    function showDiffMenu(staged, operation, text, hunkId, selectedRows, fromCommit) {
        diffMenu.staged = staged
        diffMenu.operation = operation
        diffMenu.text = text
        diffMenu.hunkId = hunkId
        diffMenu.selectedRows = selectedRows
        diffMenu.fromCommit = fromCommit === true
        diffMenu.popup()
    }

    function showLogMenu(commitId, message, fromRefLog) {
        logMenu.commitId = commitId
        logMenu.commitMessage = message
        logMenu.fromRefLog = fromRefLog === true
        logMenu.popup()
    }

    function showBranchMenu(branchName) {
        if (root.repository === null)
            return
        root.menuBranchName = branchName
        branchMenu.canRebaseOn = (branchName !== root.repository.currentBranch)
        branchMenu.canMerge = (branchName !== root.repository.currentBranch)
        branchMenu.popup()
    }

    function requestRebaseOnBranch(branchName) {
        root.confirmBranchAction(Const.rebaseOnBranchText + " " + branchName,
                                 Const.rebaseOnBranchMessage.arg(branchName),
                                 rebaseOnBranchAction, branchName)
    }

    function requestMergeBranch(branchName) {
        root.confirmBranchAction(Const.mergeBranchText + " " + branchName,
                                 Const.mergeBranchMessage.arg(branchName),
                                 mergeBranchAction, branchName)
    }

    function requestDeleteBranch(branchName) {
        root.confirmBranchAction(Const.deleteBranchText + " " + branchName,
                                 Const.deleteBranchMessage,
                                 deleteBranchAction, branchName)
    }

    function confirmBranchAction(title, message, action, targetBranchName) {
        action.targetBranchName = targetBranchName
        confirm.title = title
        confirm.messageText = message
        confirm.actionOnAccept = action
        confirm.actionOnReject = null
        confirm.open()
    }

    function panelGeometry(panelData, key, fallback) {
        if (panelData === null || panelData === undefined)
            return fallback
        var value = panelData[key]
        if (value === undefined || value === null || isNaN(Number(value)))
            return fallback
        return Number(value)
    }

    function forEachPanel(callback) {
        for (var index = 0; index < panelRepeater.count; index++) {
            var panel = panelRepeater.itemAt(index)
            if (panel !== null)
                callback(panel)
        }
    }

    function getSelectedFiles() {
        var aggregatedFiles = []
        root.forEachPanel(function(panel) {
            var selectedFiles = panel.getSelectedFiles()
            for (var fileIndex = 0; fileIndex < selectedFiles.length; fileIndex++) {
                if (aggregatedFiles.indexOf(selectedFiles[fileIndex]) === -1)
                    aggregatedFiles.push(selectedFiles[fileIndex])
            }
        })
        return aggregatedFiles
    }

    function reportMissingTab(tabId) {
        var message = Const.missingTabInLayoutText.arg(tabId)
        console.warn(message)
        if (root.controller !== null)
            root.controller.statusText = message
    }

    function activeTabIdsEqual(first, second) {
        if (first.length !== second.length)
            return false
        for (var index = 0; index < first.length; index++) {
            if (second.indexOf(first[index]) === -1)
                return false
        }
        return true
    }

    function refreshActiveTabs() {
        var collectedTabIds = []
        root.forEachPanel(function(panel) {
            if (panel.currentTabId !== undefined && panel.currentTabId !== null && panel.currentTabId !== "") {
                if (collectedTabIds.indexOf(panel.currentTabId) === -1)
                    collectedTabIds.push(panel.currentTabId)
            }
        })
        if (root.activeTabIdsEqual(root.activeTabIds, collectedTabIds) === false)
            root.activeTabIds = collectedTabIds
    }

    function hasTab(tabId) {
        var found = false
        root.forEachPanel(function(panel) {
            if (found === false && "hasTab" in panel && panel.hasTab(tabId))
                found = true
        })
        return found
    }

    function activateTab(tabId) {
        var activated = false
        root.forEachPanel(function(panel) {
            if (activated === false && panel.activateTab(tabId))
                activated = true
        })
        if (activated === false && root.hasTab(tabId) === false)
            root.reportMissingTab(tabId)
        return activated
    }

    function showTab(tabId) {
        var shown = false
        root.forEachPanel(function(panel) {
            if (shown === false && panel.showTab(tabId))
                shown = true
        })
        if (shown === false && root.hasTab(tabId) === false)
            root.reportMissingTab(tabId)
        return shown
    }

    function activateFlatFileView() {
        root.activateTab(Const.filesTabId)
    }

    function activateGraphView() {
        root.activateTab(Const.graphTabId)
    }

    function activateFileDiffView() {
        root.activateTab(Const.diffTabId)
    }

    function activateCommitDiffView() {
        if (root.activateTab(Const.commitDiffTabId) === false)
            root.activateFileDiffView()
    }

    function activateFileBlameView() {
        root.activateTab(Const.blameTabId)
    }
}

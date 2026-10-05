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

    property variant repository: null

    //--------------------------------------------------------------------------------
    // Status

    StandardPane {
        id: repositoryStatus
        objectName: "repositoryStatus"
        anchors.left: parent.left
        anchors.right: rightPart.left
        anchors.top: parent.top
        height: (Const.elementHeight * 1.5) * 2

        Item {
            id: repositoryStatusTop
            objectName: "repositoryStatusTop"
            width: parent.width
            height: parent.height * 0.5

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
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter

                color: root.repository
                       ? root.repository.repositoryStatus === CEnums.NoMerge
                         ? Material.foreground
                         : Material.background
                : "black"

                property string infoText: if (root.repository) {
                                              "%1 : ( %2 %3 / %4 %5 ) : "
                                              .arg(root.repository.repositoryTypeString)
                                              .arg(Const.aheadText)
                                              .arg(root.repository.commitCountAhead)
                                              .arg(Const.behindText)
                                              .arg(root.repository.commitCountBehind)
                                          } else ""

                text: if (root.repository) {
                          if (root.repository.repositoryStatus === CEnums.InteractiveRebase)
                              infoText + Const.interactiveRebaseProgressText
                          else if (root.repository.repositoryStatus === CEnums.Rebase)
                              infoText + Const.rebaseProgressText
                          else if (root.repository.repositoryStatus === CEnums.Merge)
                              infoText + Const.mergeProgressText
                          else infoText
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
    // File view and tools

    Item {
        id: centralPart
        objectName: "centralPart"
        anchors.top: repositoryStatus.bottom
        anchors.bottom: bottomPart.top
        anchors.left: parent.left
        anchors.right: rightPart.left

        ItemSelectionModel {
            id: flatFileSelection
            objectName: "flatFileSelection"
            model: root.repository ? root.repository.flatFileModelProxy : null

            onCurrentIndexChanged: {
                root.repository.flatFileModelProxy.handleCurrentIndex(currentIndex)
            }
        }

        FilePane {
            id: filePane
            objectName: "filePane"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: parent.width * 0.5
            anchors.margins: Const.paneMargins

            extensionParent: root
            repository: root.repository
            flatSelection: flatFileSelection

            onRequestMenu: {
                fileMenu.fileNames = fileNames
                fileMenu.popup()
            }

            onRequestDeleteFiles: root.requestDeleteFiles(fileNames)
            onRequestFileFilter: root.repository.setFileFilter(text)
            onRequestFileSortField: root.repository.setFileSortField(field)
            onRequestFileSortDirection: root.repository.setFileSortDirection(direction)
        }

        ToolPane {
            id: toolPane
            objectName: "toolPane"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: filePane.right
            anchors.right: parent.right
            anchors.margins: Const.paneMargins

            extensionParent: root
            repository: root.repository

            onRequestDiffMenu: {
                diffMenu.operation = operation
                diffMenu.text = text
                diffMenu.popup()
            }

            onRequestSaveFileDiffAsPatch: {
                root.requestPatchSave()
            }

            onRequestLogMenu: {
                logMenu.commitId = commitId
                logMenu.commitMessage = message
                logMenu.popup()
            }

            onRequestCommitDiffPrevious: root.requestCommitDiffPrevious(commitId)
            onRequestCommitDiffFrom: root.requestCommitDiffFrom(commitId)
            onRequestCommitDiffTo: root.requestCommitDiffTo(commitId)
        }
    }

    Item {
        id: rightPart
        objectName: "rightPart"
        anchors.top: parent.top
        anchors.bottom: bottomPart.top
        anchors.right: parent.right
        width: parent.width * 0.2

        BranchPane {
            id: branchPane
            objectName: "branchPane"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            repository: root.repository

            onRequestRebaseOnBranch: {
                rebaseOnBranchAction.branchName = name
                confirm.title = Const.rebaseOnBranchText + " " + name
                confirm.messageText = Const.rebaseOnBranchMessage.arg(name)
                confirm.actionOnAccept = rebaseOnBranchAction
                confirm.actionOnReject = null
                confirm.open()
            }

            onRequestMergeBranch: {
                mergeBranchAction.branchName = name
                confirm.title = Const.mergeBranchText + " " + name
                confirm.messageText = Const.mergeBranchMessage.arg(name)
                confirm.actionOnAccept = mergeBranchAction
                confirm.actionOnReject = null
                confirm.open()
            }

            onRequestDeleteBranch: {
                deleteBranchAction.branchName = name
                confirm.title = Const.deleteBranchText + " " + name
                confirm.messageText = Const.deleteBranchMessage
                confirm.actionOnAccept = deleteBranchAction
                confirm.actionOnReject = null
                confirm.open()
            }

            Action {
                id: rebaseOnBranchAction
                property string branchName: ""
                onTriggered: root.repository.rebaseOnBranch(branchName)
            }

            Action {
                id: mergeBranchAction
                property string branchName: ""
                onTriggered: root.repository.mergeBranch(branchName)
            }

            Action {
                id: deleteBranchAction
                property string branchName: ""
                onTriggered: root.repository.deleteBranch(branchName)
            }
        }
    }

    Item {
        id: bottomPart
        objectName: "bottomPart"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height * 0.4

        BranchLogPane {
            id: branchLogPane
            objectName: "branchLogPane"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: parent.width * 0.6

            extensionParent: root
            repository: root.repository

            onRequestMenu: {
                logMenu.commitId = commitId
                logMenu.commitMessage = message
                logMenu.popup()
            }

            onRequestCommitDiffPrevious: root.requestCommitDiffPrevious(commitId)
            onRequestCommitDiffFrom: root.requestCommitDiffFrom(commitId)
            onRequestCommitDiffTo: root.requestCommitDiffTo(commitId)

            onRequestTextFilter: {
                root.repository.setBranchLogFilter(text)
            }
        }

        OutputPane {
            id: outputPane
            objectName: "outputPane"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: branchLogPane.right
            anchors.right: parent.right
            anchors.margins: Const.paneMargins

            extensionParent: root
            repository: root.repository
        }
    }

    //--------------------------------------------------------------------------------
    // Menus

    DiffMenu {
        id: diffMenu
        objectName: "diffMenu"
        repository: root.repository

        onRequestCopyText: {
            root.repository.copyText(text)
        }
    }

    LogMenu {
        id: logMenu
        objectName: "logMenu"
        repository: root.repository

        onRequestCopyText: {
            root.repository.copyText(text)
        }

        onRequestCommitTagOn: {
            tagOn.commitId = commitId
            tagOn.open()
        }

        onRequestCommitBranchFrom: {
            branchFrom.commitId = commitId
            branchFrom.open()
        }

        onRequestCommitReset: {
            root.repository.commitReset(commitId)
        }

        onRequestCommitRebase: {
            root.repository.commitRebase(commitId)
        }

        onRequestCommitSquash: {
            root.repository.commitSquash(commitId)
        }

        onRequestCommitMessageChange: {
            commit.messageEnabled = true
            commit.messageText = commitMessage
            commit.showFileList = false
            commit.amend = false
            commit.commitId = commitId
            commit.open()
        }

        onRequestCommitDiffPrevious: {
            root.requestCommitDiffPrevious(commitId)
        }

        onRequestCommitDiffFrom: {
            root.requestCommitDiffFrom(commitId)
        }

        onRequestCommitDiffTo: {
            root.requestCommitDiffTo(commitId)
        }
    }

    FileMenu {
        id: fileMenu
        objectName: "fileMenu"

        onRequestDelete: root.requestDeleteFiles(fileNames)
        onRequestBlame: root.requestBlameFiles(fileNames)
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
        onTriggered: root.repository.saveFileDiffAsPatch(fileName)
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

    function requestPushAsWIP() {
        root.repository.pushAsWIP()
    }

    function requestCleanUp() {
        root.repository.cleanUp()
    }

    function requestStageAll() {
        root.repository.stageAll()
    }

    function requestUnstageAll() {
        root.repository.unstageAll()
    }

    function requestStageSelection() {
        root.repository.stageSelection(filePane.getSelectedFiles())
    }

    function requestUnstageSelection() {
        root.repository.unstageSelection(filePane.getSelectedFiles())
    }

    function requestRevertSelection() {
        root.repository.revertSelection(filePane.getSelectedFiles())
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
        root.activateFileDiffView()
    }

    function requestCommitDiffFrom(commitId) {
        root.repository.diffFromCommitId = commitId
    }

    function requestCommitDiffTo(commitId) {
        root.repository.diffToCommitId = commitId
        root.activateFileDiffView()
    }

    function requestPatchSave() {
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

    function activateFlatFileView() {
        filePane.activateFlatFileView()
    }

    function activateBranchLogView() {
        branchLogPane.activateBranchLogView()
    }

    function activateGraphView() {
        branchLogPane.activateGraphView()
    }

    function activateFileDiffView() {
        toolPane.activateFileDiffView()
    }

    function activateFileBlameView() {
        toolPane.activateFileBlameView()
    }
}

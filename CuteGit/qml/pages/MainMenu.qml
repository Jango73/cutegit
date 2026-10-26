import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQml 2.12
import CuteGit 1.0
import "../components"

MenuBar {
    id: root

    Material.elevation: 4

    property variant controller: null
    property variant repository: null
    property bool rebaseInProgress: repository
                                    ? repository.repositoryStatus === CEnums.Rebase
                                      || repository.repositoryStatus === CEnums.InteractiveRebase
                                    : false

    property alias cloneRepositoryAction: cloneRepository
    property alias openRepositoryAction: openRepository
    property alias fetchAction: fetch
    property alias pullAction: pull
    property alias pushAction: push
    property alias forcePushAction: forcePush
    property alias updateSubmodulesAction: updateSubmodules
    property alias stageSelectionAction: stageSelection
    property alias revertSelectionAction: revertSelection
    property alias commitAction: commit
    property alias amendAction: amend
    property alias saveStashAction: saveStash
    property alias popStashAction: popStash

    signal requestCloneRepository()
    signal requestOpenRepository()

    signal requestFetch()
    signal requestPull()
    signal requestPush()
    signal requestForcePush()
    signal requestPushAsWIP()

    signal requestCleanUp()
    signal requestUpdateSubmodules()
    signal requestStageAll()
    signal requestUnstageAll()
    signal requestStageSelection()
    signal requestUnstageSelection()
    signal requestRevertSelection()
    signal requestCommit()
    signal requestAmend()
    signal requestContinueRebase()
    signal requestAbortRebase()

    signal requestStashSave()
    signal requestStashPop()
    signal requestPatchApply()

    signal requestRefresh()

    signal requestHelp()
    signal requestSettings()

    Menu {
        id: repositoryMenu
        title: Const.repositoryMenuText
        width: Math.round(implicitBackgroundWidth * 4 / 3)
        delegate: ShortcutMenuItem {}

        Action {
            id: cloneRepository
            text: Const.cloneMenuText
            shortcut: "Ctrl+Shift+O"
            onTriggered: root.requestCloneRepository()
        }

        Action {
            id: openRepository
            text: Const.openMenuText
            shortcut: "Ctrl+O"
            onTriggered: root.requestOpenRepository()
        }

        Action {
            text: Const.removeMenuText
        }

        Menu {
            id: knownRepos
            width: root.width * 0.5
            title: Const.knownMenuText

            Instantiator {
                model: root.controller.knownRepositoryModel

                MenuItem {
                    text: "%1 (%2)"
                    .arg(root.controller.repositoryNameFromPath(model.display))
                    .arg(model.display)

                    onClicked: {
                        // When calling root.controller.openRepository(model.display) from here, the menu does not close...
                        // It must be missing some signal
                        openRepositoryTimer.start()
                    }

                    Timer {
                        id: openRepositoryTimer
                        repeat: false
                        interval: 500
                        onTriggered: root.controller.openRepository(model.display)
                    }
                }

                onObjectAdded: function(index, object) { knownRepos.insertItem(index, object) }
                onObjectRemoved: knownRepos.removeItem(object)
            }
        }

        MenuSeparator { }

        Action {
            text: Const.quitMenuText
            shortcut: "Ctrl+Q"
            onTriggered: root.controller.quit()
        }
    }

    Menu {
        title: Const.remoteMenuText
        width: Math.round(implicitBackgroundWidth * 4 / 3)
        delegate: ShortcutMenuItem {}

        Action {
            id: fetch
            text: Const.fetchMenuText
            shortcut: "Ctrl+F"
            enabled: root.repository ? root.repository.can(CEnums.Fetch) : false
            onTriggered: root.requestFetch()
        }

        Action {
            id: pull
            text: Const.pullMenuText
            shortcut: "Ctrl+L"
            enabled: root.repository ? root.repository.can(CEnums.Pull) && root.repository.hasPullableCommits : false
            onTriggered: root.requestPull()
        }

        Action {
            id: push
            text: Const.pushMenuText
            shortcut: "Ctrl+P"
            enabled: root.repository ? root.repository.can(CEnums.Push) && root.repository.hasPushableCommits : false
            onTriggered: root.requestPush()
        }

        Action {
            id: forcePush
            text: Const.forcePushMenuText
            shortcut: "Ctrl+Shift+P"
            enabled: root.repository ? root.repository.can(CEnums.ForcePush) && root.repository.hasPushableCommits : false
            onTriggered: root.requestForcePush()
        }

        Action {
            id: pushAsWIP
            text: Const.pushAsWIPMenuText
            shortcut: "Ctrl+W"
            enabled: root.repository ? root.repository.can(CEnums.PushasWIP) && root.repository.hasPushableCommits : false
            onTriggered: root.requestPushAsWIP()
        }
    }

    Menu {
        title: Const.localMenuText
        width: Math.round(implicitBackgroundWidth * 4 / 3)
        delegate: ShortcutMenuItem {}

        Action {
            id: cleanUp
            text: Const.cleanUpMenuText
            shortcut: "Ctrl+N"
            enabled: root.repository
                     ? root.repository.can(CEnums.CleanUp)
                     : false
            onTriggered: root.requestCleanUp()
        }

        Action {
            id: updateSubmodules
            text: Const.updateSubmodulesMenuText
            shortcut: "Ctrl+U"
            enabled: root.repository
                     ? root.repository.can(CEnums.UpdateSubmodules)
                     : false
            onTriggered: root.requestUpdateSubmodules()
        }

        MenuSeparator { }

        Action {
            text: Const.stageAllMenuText
            shortcut: "Ctrl+Shift++"
            enabled: root.repository
                     ? root.repository.can(CEnums.Stage) && root.repository.hasModifiedFiles
                     : false
            onTriggered: root.requestStageAll()
        }

        Action {
            text: Const.unstageAllMenuText
            shortcut: "Ctrl+Shift+-"
            enabled: root.repository
                     ? root.repository.can(CEnums.Unstage) && root.repository.hasModifiedFiles
                     : false
            onTriggered: root.requestUnstageAll()
        }

        Action {
            id: stageSelection
            text: Const.stageSelectionMenuText
            shortcut: "Ctrl++"
            enabled: root.repository
                     ? root.repository.can(CEnums.Stage) && root.repository.hasModifiedFiles
                     : false
            onTriggered: root.requestStageSelection()
        }

        Action {
            text: Const.unstageSelectionMenuText
            shortcut: "Ctrl+-"
            enabled: root.repository
                     ? root.repository.can(CEnums.Unstage) && root.repository.hasModifiedFiles
                     : false
            onTriggered: root.requestUnstageSelection()
        }

        MenuSeparator { }

        Action {
            id: revertSelection
            text: Const.revertSelectionMenuText
            shortcut: "Ctrl+Z"
            enabled: root.repository
                     ? root.repository.hasModifiedFiles
                     : false
            onTriggered: root.requestRevertSelection()
        }

        MenuSeparator { }

        Action {
            id: commit
            text: Const.commitMenuText
            shortcut: "Ctrl+C"
            enabled: root.repository
                     ? root.repository.can(CEnums.Commit) && root.repository.hasCommitableFiles
                     : false
            onTriggered: root.requestCommit()
        }

        Action {
            id: amend
            text: Const.amendMenuText
            shortcut: "Ctrl+M"
            enabled: root.repository
                     ? root.repository.can(CEnums.Amend) &&
                       (
                           (root.repository.repositoryStatus !== CEnums.NoMerge)
                           ||
                           (root.repository.repositoryStatus === CEnums.NoMerge && root.repository.hasCommitableFiles)
                           )
                     : false
            onTriggered: root.requestAmend()
        }

        MenuSeparator { }

        Action {
            text: Const.continueRebaseMenuText
            shortcut: "Ctrl+R"
            enabled: root.repository ? root.rebaseInProgress && root.repository.can(CEnums.ContinueRebase) : false
            onTriggered: root.requestContinueRebase()
        }

        Action {
            text: Const.abortRebaseMenuText
            shortcut: "Ctrl+T"
            enabled: root.repository ? root.rebaseInProgress && root.repository.can(CEnums.AbortRebase) : false
            onTriggered: root.requestAbortRebase()
        }
    }

    Menu {
        title: Const.stashAndPatchMenuText
        width: Math.round(implicitBackgroundWidth * 4 / 3)
        delegate: ShortcutMenuItem {}

        Action {
            id: saveStash
            text: Const.saveStashMenuText
            shortcut: "Ctrl+S"
            enabled: root.repository
                     ? root.repository.can(CEnums.Stash) && root.repository.hasModifiedFiles
                     : false
            onTriggered: root.requestStashSave()
        }

        Action {
            id: popStash
            text: Const.popStashMenuText
            shortcut: "Ctrl+Shift+S"
            enabled: root.repository ? root.repository.can(CEnums.Stash) : false
            onTriggered: root.requestStashPop()
        }

        MenuSeparator { }

        Action {
            id: applyPatch
            text: Const.applyPatchMenuText
            shortcut: "Ctrl+E"
            enabled: root.repository
                     ? root.repository.can(CEnums.ApplyPatch)
                     : false
            onTriggered: root.requestPatchApply()
        }
    }

    /*
      TODO:
    Menu {
        title: Const.submoduleMenuText

        MenuItem {
            text: Const.addSubmoduleMenuText
            enabled: false
        }

        MenuItem {
            text: Const.updateSubmoduleMenuText
            enabled: false
        }

        MenuItem {
            text: Const.removeSubmoduleMenuText
            enabled: false
        }
    }
    */

    Menu {
        title: Const.viewMenuText
        width: Math.round(implicitBackgroundWidth * 4 / 3)
        delegate: ShortcutMenuItem {}

        Action {
            text: Const.refreshMenuText
            shortcut: "F5"
            onTriggered: root.requestRefresh()
        }

        MenuSeparator { }

        Action {
            text: Const.showAllMenuText
            shortcut: "Ctrl+Shift+A"

            onTriggered: {
                root.controller.showClean = true
                root.controller.showAdded = true
                root.controller.showModified = true
                root.controller.showRenamed = true
                root.controller.showDeleted = true
                root.controller.showMissing = true
                root.controller.showUntracked = true
                root.controller.showIgnored = true
            }
        }

        MenuSeparator { }

        Action {
            text: Const.showCleanMenuText
            shortcut: "Alt+1"
            checkable: true
            checked: root.controller.showClean
            onTriggered: root.controller.showClean = !root.controller.showClean
        }

        Action {
            text: Const.showAddedMenuText
            shortcut: "Alt+2"
            checkable: true
            checked: root.controller.showAdded
            onTriggered: root.controller.showAdded = !root.controller.showAdded
        }

        Action {
            text: Const.showModifiedMenuText
            shortcut: "Alt+3"
            checkable: true
            checked: root.controller.showModified
            onTriggered: root.controller.showModified = !root.controller.showModified
        }

        Action {
            text: Const.showRenamedMenuText
            shortcut: "Alt+4"
            checkable: true
            checked: root.controller.showRenamed
            onTriggered: root.controller.showRenamed = !root.controller.showRenamed
        }

        Action {
            text: Const.showDeletedMenuText
            shortcut: "Alt+5"
            checkable: true
            checked: root.controller.showDeleted
            onTriggered: root.controller.showDeleted = !root.controller.showDeleted
        }

        Action {
            text: Const.showMissingMenuText
            shortcut: "Alt+6"
            checkable: true
            checked: root.controller.showMissing
            onTriggered: root.controller.showMissing = !root.controller.showMissing
        }

        Action {
            text: Const.showUntrackedMenuText
            shortcut: "Alt+7"
            checkable: true
            checked: root.controller.showUntracked
            onTriggered: root.controller.showUntracked = !root.controller.showUntracked
        }

        Action {
            text: Const.showIgnoredMenuText
            shortcut: "Alt+8"
            checkable: true
            checked: root.controller.showIgnored
            onTriggered: root.controller.showIgnored = !root.controller.showIgnored
        }
    }

    Menu {
        title: Const.optionsMenuText
        width: Math.round(implicitBackgroundWidth * 4 / 3)
        delegate: ShortcutMenuItem {}

        Action {
            text: Const.helpMenuText
            shortcut: "F1"
            onTriggered: root.requestHelp()
        }

        Action {
            text: Const.settingsMenuText
            shortcut: "Ctrl+,"
            onTriggered: root.requestSettings()
        }
    }
}

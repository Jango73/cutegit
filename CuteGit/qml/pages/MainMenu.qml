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
    property variant materialTheme: 0
    property bool rebaseInProgress: repository
                                    ? repository.repositoryStatus === CEnums.Rebase
                                      || repository.repositoryStatus === CEnums.InteractiveRebase
                                    : false

    property alias cloneRepositoryAction: cloneRepository
    property alias openRepositoryAction: openRepository
    property alias fetchAction: fetch
    property alias pullAction: pull
    property alias pushAction: push
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
    signal requestPushAsWIP()

    signal requestCleanUp()
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

    signal requestDarkTheme()
    signal requestLightTheme()
    signal requestHelp()
    signal requestLanguage(var language)

    Menu {
        id: repositoryMenu
        title: Const.repositoryMenuText

        Action {
            id: cloneRepository
            text: Const.cloneMenuText
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
            id: pushAsWIP
            text: Const.pushAsWIPMenuText
            shortcut: "Ctrl+W"
            enabled: root.repository ? root.repository.can(CEnums.PushasWIP) && root.repository.hasPushableCommits : false
            onTriggered: root.requestPushAsWIP()
        }
    }

    Menu {
        title: Const.localMenuText

        Action {
            id: cleanUp
            text: Const.cleanUpMenuText
            shortcut: "Ctrl+N"
            enabled: root.repository
                     ? root.repository.can(CEnums.CleanUp)
                     : false
            onTriggered: root.requestCleanUp()
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
                           (root.repository.repositoryStatus === CEnums.NoMerge && root.repository.hasCommitableFiles && root.repository.commitCountAhead > 0)
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

        Action {
            id: saveStash
            text: Const.saveStashMenuText
            enabled: root.repository
                     ? root.repository.can(CEnums.Stash) && root.repository.hasModifiedFiles
                     : false
            onTriggered: root.requestStashSave()
        }

        Action {
            id: popStash
            text: Const.popStashMenuText
            enabled: root.repository ? root.repository.can(CEnums.Stash) : false
            onTriggered: root.requestStashPop()
        }

        MenuSeparator { }

        Action {
            id: applyPatch
            text: Const.applyPatchMenuText
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

        Action {
            text: Const.refreshMenuText
            shortcut: "F5"
            onTriggered: root.requestRefresh()
        }

        MenuSeparator { }

        MenuItem {
            text: Const.showAllMenuText

            onClicked: {
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

        MenuItem {
            text: Const.showCleanMenuText
            checkable: true
            checked: root.controller.showClean
            onClicked: root.controller.showClean = !root.controller.showClean
        }

        MenuItem {
            text: Const.showAddedMenuText
            checkable: true
            checked: root.controller.showAdded
            onClicked: root.controller.showAdded = !root.controller.showAdded
        }

        MenuItem {
            text: Const.showModifiedMenuText
            checkable: true
            checked: root.controller.showModified
            onClicked: root.controller.showModified = !root.controller.showModified
        }

        MenuItem {
            text: Const.showRenamedMenuText
            checkable: true
            checked: root.controller.showRenamed
            onClicked: root.controller.showRenamed = !root.controller.showRenamed
        }

        MenuItem {
            text: Const.showDeletedMenuText
            checkable: true
            checked: root.controller.showDeleted
            onClicked: root.controller.showDeleted = !root.controller.showDeleted
        }

        MenuItem {
            text: Const.showMissingMenuText
            checkable: true
            checked: root.controller.showMissing
            onClicked: root.controller.showMissing = !root.controller.showMissing
        }

        MenuItem {
            text: Const.showUntrackedMenuText
            checkable: true
            checked: root.controller.showUntracked
            onClicked: root.controller.showUntracked = !root.controller.showUntracked
        }

        MenuItem {
            text: Const.showIgnoredMenuText
            checkable: true
            checked: root.controller.showIgnored
            onClicked: root.controller.showIgnored = !root.controller.showIgnored
        }
    }

    Menu {
        title: Const.optionsMenuText

        Action {
            text: Const.helpMenuText
            onTriggered: root.requestHelp()
        }

        Menu {
            title: Const.themeMenuText

            MenuItem {
                text: Const.darkThemeMenuText
                checkable: true
                checked: root.materialTheme === Material.Dark
                onClicked: root.requestDarkTheme()
            }

            MenuItem {
                text: Const.lightThemeMenuText
                checkable: true
                checked: root.materialTheme === Material.Light
                onClicked: root.requestLightTheme()
            }
        }

        Menu {
            id: language
            title: Const.languageMenuText

            Instantiator {
                model: root.controller.langModel

                MenuItem {
                    text: model.display
                    checkable: true
                    checked: root.controller.language === model.display
                    onClicked: root.requestLanguage(model.display)
                }

                onObjectAdded: function(index, object) { language.insertItem(index, object) }
                onObjectRemoved: language.removeItem(object)
            }
        }

        Menu {
            id: layout
            title: Const.layoutText

            Instantiator {
                model: root.controller.layoutNames

                MenuItem {
                    text: modelData
                    checkable: true
                    checked: root.controller.currentLayoutIndex === index
                    onClicked: root.controller.currentLayoutIndex = index
                }

                onObjectAdded: function(index, object) { layout.insertItem(index, object) }
                onObjectRemoved: layout.removeItem(object)
            }
        }
    }
}

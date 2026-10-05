pragma Singleton
import QtQuick 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Window 2.12

Item {
    // Constants
    readonly property string themeMaterialDark: "MaterialDark"
    readonly property string themeMaterialLight: "MaterialLight"

    // Main font
    readonly property string mainFontFamily: "Calibri"
    readonly property int mainFontSize: 12 // (Screen.pixelDensity * 3)

    // Other fonts
    readonly property string fixedWidthFontFamily: "Courier"
    readonly property int fixedWidthFontSize: mainFontSize

    // Colors
    readonly property color transparent: "transparent"

    readonly property color fileStagedColor: "#80008000"
    readonly property color fileModifiedColor: "#80800000"
    readonly property color fileRenamedColor: "#80804000"
    readonly property color fileAddedColor: "#80000080"
    readonly property color fileDeletedColor: "#80804000"

    readonly property color labelLocalColor: "aquamarine"
    readonly property color labelRemoteColor: "lightsalmon"
    readonly property color labelTagColor: "deepskyblue"

    // Margins and padding
    readonly property int mainPadding: mainFontSize * 0.6666
    readonly property int mainRadius: mainPadding * 0.5
    readonly property int smallPadding: mainPadding / 2
    readonly property int verySmallPadding: smallPadding / 2
    readonly property int panePadding: smallPadding
    readonly property int paneMargins: smallPadding
    readonly property int paneElevation: 2
    readonly property int popupElevation: 8
    readonly property int elementHeight: mainFontSize * 1.5
    readonly property int microButtonWidth: elementHeight * 1.2
    readonly property int microButtonHeight: elementHeight * 1.8
    readonly property int listViewItemHeight: elementHeight + smallPadding
    readonly property int buttonHeight: mainFontSize * 3

    readonly property real popupWidthNorm: 0.50
    readonly property real popupHeightNorm: 0.75

    readonly property real popupWidthSmall: 0.25
    readonly property real popupHeightSmall: 0.25

    // Durations
    readonly property int componentFadingDuration: 200
    readonly property int toolTipDelay: 1000
    readonly property int toolTipTimeout: 5000

    // Text
    readonly property string repositoryMenuText: qsTr("&Repository")
    readonly property string cloneMenuText: qsTr("&Clone / checkout")
    readonly property string openMenuText: qsTr("&Open")
    readonly property string removeMenuText: qsTr("&Remove")
    readonly property string knownMenuText: qsTr("&Known")
    readonly property string quitMenuText: qsTr("&Quit")

    readonly property string remoteMenuText: qsTr("Re&mote")
    readonly property string fetchMenuText: qsTr("&Fetch")
    readonly property string pullMenuText: qsTr("&Pull")
    readonly property string pushMenuText: qsTr("Pus&h")
    readonly property string pushAsWIPMenuText: qsTr("Push as &WIP")

    readonly property string cleanUpMenuText: qsTr("Clea&n up")
    readonly property string localMenuText: qsTr("&Local")
    readonly property string stageAllMenuText: qsTr("Stage a&ll")
    readonly property string unstageAllMenuText: qsTr("Unsta&ge all")
    readonly property string stageSelectionMenuText: qsTr("&Stage selection")
    readonly property string unstageSelectionMenuText: qsTr("&Unstage selection")
    readonly property string revertSelectionMenuText: qsTr("&Revert selection")
    readonly property string commitMenuText: qsTr("&Commit")
    readonly property string amendMenuText: qsTr("&Amend")
    readonly property string continueRebaseMenuText: qsTr("C&ontinue rebase")
    readonly property string abortRebaseMenuText: qsTr("Abor&t rebase")

    readonly property string stashAndPatchMenuText: qsTr("Stash && patch")
    readonly property string saveStashMenuText: qsTr("&Save stash")
    readonly property string popStashMenuText: qsTr("&Pop stash")
    readonly property string applyPatchMenuText: qsTr("&Apply patch")

    readonly property string submoduleMenuText: qsTr("Submodules")
    readonly property string addSubmoduleMenuText: qsTr("&Add...")
    readonly property string updateSubmoduleMenuText: qsTr("&Update...")
    readonly property string removeSubmoduleMenuText: qsTr("&Remove...")

    readonly property string viewMenuText: qsTr("&View")
    readonly property string refreshMenuText: qsTr("&Refresh")
    readonly property string showAllMenuText: qsTr("Show a&ll")
    readonly property string showCleanMenuText: qsTr("Show &clean")
    readonly property string showAddedMenuText: qsTr("Show &added")
    readonly property string showModifiedMenuText: qsTr("Show &modified")
    readonly property string showRenamedMenuText: qsTr("Show &renamed")
    readonly property string showDeletedMenuText: qsTr("Show &deleted")
    readonly property string showMissingMenuText: qsTr("Show mi&ssing")
    readonly property string showUntrackedMenuText: qsTr("Show &untracked")
    readonly property string showIgnoredMenuText: qsTr("Show &ignored")

    readonly property string optionsMenuText: qsTr("&Options")
    readonly property string helpMenuText: qsTr("&Help")
    readonly property string themeMenuText: qsTr("&Theme")
    readonly property string darkThemeMenuText: qsTr("Dark")
    readonly property string lightThemeMenuText: qsTr("Light")
    readonly property string languageMenuText: qsTr("&Language")

    readonly property string fileDeleteMenuText: qsTr("&Delete")
    readonly property string fileBlameMenuText: qsTr("&Blame")

    readonly property string logCopyCommitIdMenuText: qsTr("&Copy commit ID")
    readonly property string logCopyCommitMessageMenuText: qsTr("Copy commit m&essage")
    readonly property string logCreateTagMenuText: qsTr("Create ta&g here")
    readonly property string logCreateBranchMenuText: qsTr("&Branch from here")
    readonly property string logResetMenuText: qsTr("Reset &to here")
    readonly property string logRebaseMenuText: qsTr("&Rebase here")
    readonly property string logSquashMenuText: qsTr("&Squash")
    readonly property string logChangeMessageMenuText: qsTr("Change &message")
    readonly property string logDiffPreviousCommitMenuText: qsTr("&View diff with previous commit")
    readonly property string logMarkDiffFrom: qsTr("Mark as diff from")
    readonly property string logMarkDiffTo: qsTr("Mark as diff to")

    readonly property string diffCopyLineText: qsTr("&Copy line")

    readonly property string okText: qsTr("OK")
    readonly property string logText: qsTr("Log")
    readonly property string tagText: qsTr("Tag")
    readonly property string sortText: qsTr("Sort")
    readonly property string nameText: qsTr("Name")
    readonly property string tagsText: qsTr("Tags")
    readonly property string flatText: qsTr("Flat")
    readonly property string diffText: qsTr("Diff")
    readonly property string pushText: qsTr("Push")
    readonly property string pullText: qsTr("Pull")
    readonly property string closeText: qsTr("Close")
    readonly property string clearText: qsTr("Clear")
    readonly property string filesText: qsTr("Files")
    readonly property string stageText: qsTr("Stage")
    readonly property string mergeText: qsTr("Merge")
    readonly property string graphText: qsTr("Graph")
    readonly property string amendText: qsTr("Amend")
    readonly property string fetchText: qsTr("Fetch")
    readonly property string blameText: qsTr("Blame")
    readonly property string localText: qsTr("Local")
    readonly property string aheadText: qsTr("Ahead")
    readonly property string behindText: qsTr("Behind")
    readonly property string filterText: qsTr("Filter")
    readonly property string revertText: qsTr("Revert")
    readonly property string cancelText: qsTr("Cancel")
    readonly property string outputText: qsTr("Output")
    readonly property string browseText: qsTr("Browse")
    readonly property string deleteText: qsTr("Delete")
    readonly property string commitText: qsTr("Commit")
    readonly property string remoteText: qsTr("Remote")
    readonly property string refLogText: qsTr("Ref log")
    readonly property string messageText: qsTr("Message")
    readonly property string loadingText: qsTr("Loading...")
    readonly property string generalText: qsTr("General")
    readonly property string branchesText: qsTr("Branches")
    readonly property string switchToText: qsTr("Switch to")
    readonly property string rebaseOnText: qsTr("Rebase on")
    readonly property string amendingText: qsTr("Amending. The changes will be appended to the last commit...")
    readonly property string shortcutsText: qsTr("Shortcuts")
    readonly property string branchLogText: qsTr("Branch log")
    readonly property string helpTitleText: qsTr("Help")
    readonly property string listEmptyText: qsTr("List empty")
    readonly property string expandAllText: qsTr("Expand all")
    readonly property string deleteFileText: qsTr("Delete file")
    readonly property string branchFromText: qsTr("Branch from")
    readonly property string collapseAllText: qsTr("Collapse all")
    readonly property string mergeBranchText: qsTr("Merge branch")
    readonly property string informationText: qsTr("Information")
    readonly property string saveAsPatchText: qsTr("Save as patch")
    readonly property string statusChangeText: qsTr("...")
    readonly property string deleteBranchText: qsTr("Delete branch")
    readonly property string repositoriesText: qsTr("Repositories")
    readonly property string repositoryURLText: qsTr("Repository URL")
    readonly property string statusHistoryText: qsTr("Status history")
    readonly property string mergeProgressText: qsTr("Merge in progress...")
    readonly property string allFilesCleanText: qsTr("All files are clean")
    readonly property string rebaseOnBranchText: qsTr("Rebase on branch")
    readonly property string rebaseProgressText: qsTr("Rebase in progress...")
    readonly property string cloneRepositoryText: qsTr("Clone repository")
    readonly property string enterMessageHereText: qsTr("Enter your message here...")
    readonly property string destinationFolderText: qsTr("Destination folder")
    readonly property string selectRepositoryFolderText: qsTr("Select repository folder")
    readonly property string selectPatchToSaveText: qsTr("Select file for patch")
    readonly property string selectPatchToApplyText: qsTr("Select patch to apply")
    readonly property string enterBranchNameHereText: qsTr("Enter branch name here...")
    readonly property string changeCommitMessageText: qsTr("Change commit message")
    readonly property string stagedFilesForCommitText: qsTr("Staged files for commit")
    readonly property string interactiveRebaseProgressText: qsTr("Interactive rebase in progress...")

    readonly property string emptyRepositoryTabText: qsTr(
                                                "There is no open repository at the moment.\n" +
                                                "Please open or clone a repository to fill this view."
                                                )

    readonly property string rebaseOnBranchMessage: qsTr(
                                            "You are about to rebase on %1.\n\n" +
                                            "Are you sure you want to do this?"
                                            )

    readonly property string mergeBranchMessage: qsTr(
                                            "You are about to merge %1 on the working directory.\n\n" +
                                            "Are you sure you want to do this?"
                                            )

    readonly property string deleteBranchMessage: qsTr(
                                             "You are about to delete a branch.\n\n" +
                                             "Are you sure you want to do this? It cannot be undone."
                                             )

    readonly property string deleteFileMessage: qsTr(
                                           "Are you sure you want to delete this(ese) file(s)?\n\n" +
                                           "%1"
                                           )

    readonly property string setLanguageText: qsTr(
                                         "The language selection (%1) will be applied when the application is restarted."
                                         )

    readonly property string cannotSetLanguageText: qsTr(
                                         "The language selection (%1) could not be set."
                                         )

    readonly property string copyrightText: qsTr(
                                  "CuteGit - version %1\n" +
                                  "Copyright (c) 2019 Jango73\n" +
                                  "Some icons are copyright Freepik from www.flaticon.com\n"
                                  )

    readonly property string generalHelpText: qsTr(
                                         "Repository status\n" +
                                         "\n" +
                                         "The first line contains the type of repository, the number of commits ahead and behind the current HEAD.\n" +
                                         "The secoond line contains full path of the repository and the current branch.\n" +
                                         "\n" +
                                         "File view\n" +
                                         "\n" +
                                         "Staged files are shown in green, unstaged ones in red.\n" +
                                         "The symbols next to file names are:\n" +
                                         "*\tThe file is modified\n" +
                                         "+\tThe file is being added\n" +
                                         "-\tThe file is being removed\n" +
                                         "=\tThe file is being renamed\n" +
                                         "?\tThe file is untracked\n" +
                                         "!\tThe file is ignored\n" +
                                         "???\tThe file is missing\n" +
                                         "\n" +
                                         "Diff views\n" +
                                         "\n" +
                                         "Green is for new lines, red for deleted lines.\n" +
                                         "\n" +
                                         "Log views\n" +
                                         "\n" +
                                         "The first line of an item contains part of the commit message.\n" +
                                         "The second line contains the author, the date and all refs that point to this commit.\n" +
                                         "\n" +
                                         "Graph view\n" +
                                         "\n" +
                                         "It contains the same information as a log view.\n" +
                                         "\n" +
                                         "Ref log\n" +
                                         "\n" +
                                         "The first line of an item contains the action done, the second line contains the index.\n"
                                         )

    readonly property string tipExpandButton: qsTr("Expands or shrinks the view.")
    readonly property string tipBrowseButton: qsTr("Opens a file system browser.")
    readonly property string tipSortDirectionButton: qsTr("Toggles ascendant and descendant.")
    readonly property string tipFileFilter: qsTr("Show only files matching this text. When text is empty, all files are shown.")
    readonly property string tipLineFilter: qsTr("Show only lines matching this text. When text is empty, all lines are shown.")

    readonly property string logLabelFrom: "[=>] "
    readonly property string logLabelTo: "[<=] "

    //
    readonly property string staged: "X"
    readonly property string statusClean: " "
    readonly property string statusModified: "M"
    readonly property string statusAdded: "+"
    readonly property string statusDeleted: "-"
    readonly property string statusRenamed: "="

    // Icons

    // By Freepik from www.flaticon.com
    readonly property string deleteIcon: "qrc:/qml/images/garbage-container.svg"

    readonly property string cloneIcon: "qrc:/qml/images/copy-documents-option.svg"
    readonly property string openIcon: "qrc:/qml/images/open-folder-black-and-white-variant"
    readonly property string pushIcon: "qrc:/qml/images/up-arrow.svg"
    readonly property string pullIcon: "qrc:/qml/images/down.svg"
    readonly property string fetchIcon: "qrc:/qml/images/loop.svg"
    readonly property string stageIcon: "qrc:/qml/images/check.svg"
    readonly property string revertIcon: deleteIcon
    readonly property string commitIcon: "qrc:/qml/images/seal.svg"
    readonly property string amendIcon: "qrc:/qml/images/plus-sign.svg"
    readonly property string saveStashIcon: "qrc:/qml/images/clipboard.svg"
    readonly property string popStashIcon: "qrc:/qml/images/blank.svg"
    readonly property string exitIcon: "qrc:/qml/images/door-exit.svg"

    // Models

    readonly property ListModel shortcutModel: ListModel {
        ListElement { name: qsTr("Open a repository"); sequence: "Control O" }
        ListElement { name: qsTr("Quit"); sequence: "Control Q" }
        ListElement { name: ""; sequence: "" }
        ListElement { name: qsTr("Refresh"); sequence: "F5" }
        ListElement { name: ""; sequence: "" }
        ListElement { name: qsTr("Stage all"); sequence: "Control Shift +" }
        ListElement { name: qsTr("Unstage all"); sequence: "Control Shift -" }
        ListElement { name: qsTr("Toggle staged"); sequence: "Space" }
        ListElement { name: qsTr("Stage selection"); sequence: "Control +" }
        ListElement { name: qsTr("Unstage selection"); sequence: "Control -" }
        ListElement { name: qsTr("Revert selection"); sequence: "Control Z" }
        ListElement { name: qsTr("Commit"); sequence: "Control C" }
        ListElement { name: qsTr("Amend"); sequence: "Control M" }
        ListElement { name: qsTr("Continue rebase"); sequence: "Control R" }
        ListElement { name: qsTr("Abort rebase"); sequence: "Control T" }
        ListElement { name: ""; sequence: "" }
        ListElement { name: qsTr("Fetch"); sequence: "Control F" }
        ListElement { name: qsTr("Pull"); sequence: "Control L" }
        ListElement { name: qsTr("Push"); sequence: "Control P" }
        ListElement { name: ""; sequence: "" }
        ListElement { name: qsTr("Activate file view"); sequence: "Alt Shift F" }
        ListElement { name: qsTr("Activate graph view"); sequence: "Alt Shift G" }
        ListElement { name: qsTr("Activate branch log view"); sequence: "Alt Shift L" }
        ListElement { name: qsTr("Activate diff view"); sequence: "Alt Shift D" }
        ListElement { name: qsTr("Activate blame view"); sequence: "Alt Shift B" }
        ListElement { name: ""; sequence: "" }
        ListElement { name: qsTr("In log view"); sequence: "" }
        ListElement { name: qsTr("Diff with previous commit"); sequence: "Space" }
        ListElement { name: qsTr("Mark file for diff"); sequence: "Enter" }
    }
}

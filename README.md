# CuteGit

[![Build](https://github.com/Jango73/cutegit/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/Jango73/cutegit/actions/workflows/build.yml)

![alt text](https://github.com/Jango73/cutegit/blob/main/Media/Screenshot01.jpg)

This is a simple multi-document UI for versioning system executables (Git, Gerrit, Mercurial, Subversion).
It is not meant to compete with paywares, just provide minimal versioning functionality.

## Dependencies

* Qt 6 development packages (exact list in `scripts/linux/deps.sh`)
* `cmake` and `ninja`
* `qt-plus` submodule (https://github.com/Jango73/qt-plus), initialized by `scripts/linux/deps.sh`
* At runtime, the versioning executables driven by the UI (`git`, plus `hg` and `svn` for those repository types)

## Building

* Install dependencies and initialize submodules (prompts for `sudo`):
```
./scripts/linux/deps.sh
```
* Build the application (the needed `qt-plus` sources compile in, no separate library build):
```
./scripts/linux/build.sh [--debug|--release]
```
* Run the automated tests (local CI, same as GitHub CI):
```
./scripts/linux/ci-local.sh [--debug|--release]
```
* Run the application:
```
./scripts/linux/run.sh [--debug|--release]
```
* Build the Release packages (`.deb` + `.tar.gz`, copied to `dist/`):
```
./scripts/linux/package.sh
```

## Things it does

* Open one or more repositories, clone from URL
* Refresh view

### Git and Gerrit

* Show repository status (clean, merge, rebase, ...) with ahead/behind counts
* Show repository files as a flat list or a tree, colored according to status, with filter and sorting
* Show the branches and tags
* Show the branch log and the graphical commit graph
* Show the reflog
* Show the selected file's log
* Show staged and unstaged diffs, stage/unstage whole files, hunks or single lines
* Show the output of the executed commands
* Switch current branch, create a branch from a commit, delete a branch
* Stage, unstage and revert files
* Delete files (`git rm`)
* Commit, amend, change a commit message, squash a commit
* Fetch, pull, push and force push (Gerrit: push for review or as WIP)
* Rebase a branch on another one, rebase on a commit, continue and abort rebase
* Merge a branch into the current one
* Reset the current branch on a commit
* Save and pop stash
* Tag a commit
* View the diff of a commit and the diff with its previous commit
* Blame a file
* Save a file diff as a patch file and apply patch files
* Clean up the repository

### Mercurial

* Show the branches
* Show repository files as a flat list or a tree, colored according to status
* Show the branch log and the global graph
* Show the selected file's log
* Mark files as staged / unstaged
* Commit, amend, branch from a commit
* Pull, push and force push

### Subversion

* Show repository files as a flat list or a tree, colored according to status
* Show the branches (trunk + branches folder)
* Show the branch log and the global graph
* Show the selected file's log
* Show the unstaged diffs in the selected item (folder or file)
* Schedule new files (add) and unschedule them
* Revert files
* Commit
* Update (pull)
* Switch current branch
* Branch and tag from a revision
* View the diff of a revision
* Blame

## Classes

### CuteGit

* Registers types
* Creates application controller
* Creates QML engine

### CController

* Owns the known and open repositories and the current repository
* Opens, clones and removes repositories, quits the application

### CRepository

* Represents one open repository
* Owns the repository status, the models (files, branches, log, graph, diff, blame, output) and the versioning backend
* Exposes the versioning actions to QML: stage, unstage, commit, push, ...

### CRepositoryModel

* List model of the open repositories

### CBranch

* The properties of a branch

### CBranchModel

* List model of the branches

### CLabel

* A marker on a commit (local branch, remote branch, tag)

### CLabelModel

* List model of the commit labels

### CTreeFileModel

* Inherits QFileSystemModel
* Shows changed files as a tree
* Provides access to the repository files

### CTreeFileModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CTreeFileModel
* Enables filtering files on
  * file name
  * status property

### CFlatFileModel

* Inherits QAbstractListModel
* Shows changed files as a list
* Provides access to the repository files

### CFlatFileModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CFlatFileModel
* Enables filtering files on
  * file name
  * status property

### CStagedFileModelProxy

* Proxy exposing the staged files

### CBaseListModel

* Minimal list-model interface exposed to QML (count, get, append, remove, move)

### CRepoFile

* The properties of a repository file
  * status
  * staged / unstaged flag

### CCommands

* A (kind of) interface for a versioning system
* Makes the versioning system abstract

### CExecution

* Runs the versioning executables and reports process output

### CGitCommands

* Inherits CCommands
* Implements Git versioning commands

### CGerritCommands

* Inherits CCommands
* Implements Gerrit commands (push for review, push as WIP)

### CSvnCommands

* Inherits CCommands
* Implements SVN versioning commands

### CHgCommands

* Inherits CCommands
* Implements Mercurial versioning commands

### CLogLine

* A line of log : commit id, date, author, message

### CLogLineCollection

* An owned collection of log lines

### CLogModel

* A collection of log lines

### CLogModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CLogModel
* Enables filtering a log on
  * commit author
  * commit message

### CGraphLine

* A line of the commit graph

### CGraphLineCollection

* An owned collection of graph lines

### CGraphModel

* The commit graph model

### CGraphLayout

* Computes the commit graph layout

### CDiffLine

* A line of diff between two files

### CDiffModel

* A collection of CDiffLine

### CDiffModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CDiffModel
* Enables filtering a diff

### CDiffConstants

* Shared diff constants

### CPatchBuilder

* Builds a patch from selected diff lines and hunks

### CLayout

* Loads the descriptive UI layouts (`layouts.json`)

### CUtils

* Miscellaneous helpers

### Strings

* Shared user-visible strings

### CEnums

* Shared enumerations and repository capabilities exposed to QML

## Code summary

`CuteGit/sources` + `CuteGit/qml`: 146 files (35 C++, 38 C++ headers, 73 QML), 20402 lines (10025 C++, 3997 headers, 6380 QML).

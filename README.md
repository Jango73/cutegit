# CuteGit

![alt text](https://github.com/Jango73/CuteGit/blob/master/Media/Screenshot01.jpg)

This is a simple multi-document UI for the Git executable (and other versioning systems in a near future).
It is not meant to compete with paywares, just provide minimal versioning functionality.

## Dependencies

* Qt 5.13
* QtCreator 4.9.1
* qt-plus - Copyright myself
  * https://github.com/Jango73/qt-plus

## Building

* Make sure to update submodules
```
    git submodule init
    git submodule update
```
* Open /CuteGit.pro in QtCreator
* Build and run

## Things it does

* Open one or more repositories
* Refresh view
* For repositories of type Git and Gerrit:
  * Show repository status (clean, merge, rebase, ...)
  * Show repository files as a flat list, colored according to status
  * Show the branches and tags
  * Show the branch log
  * Show the graph as output by Git (might be graphical some day)
  * Show the selected file's log
  * Show the unstaged diffs in the selected item (folder or file)
  * Show the output of the executed commands
  * Change current branch
  * Mark files as staged / unstaged
  * Remove files form tracking
  * Revert files
  * Commit
  * Amend (on interactive rebase, commit is always amend)
  * Fetch, pull and push
  * Modify a commit message
  * Rebase current branch on another one
  * Merge current branch with another one
  * Begin rebase on a commit, continue and abort rebase
  * Reset on a commit
  * Save and pop stash (no selectable stash yet)
  * Branch from a commit
  * Tag a commit
  * View diff with previous commit
  * View diff between any two commits
* For repositories of type Mercurial:
  * Show the branches
  * Show repository files as a flat list, colored according to status
  * Mark files as staged / unstaged
  * Show the branch log
  * Show the selected file's log
  * Commit
  * Branch from a commit
* For repositories of type SVN:
  * Show repository files as a flat list, colored according to status
  * Show the branches (trunk + branches folder)
  * Show the branch log
  * Show the selected file's log
  * Show the unstaged diffs in the selected item (folder or file)
  * Schedule new files (add) and unschedule them
  * Revert files
  * Commit
  * Update (pull)
  * Switch current branch
  * Branch and tag from a revision
  * View diff of a revision and between two revisions
  * Blame

## Classes

### CuteGit

* Registers types
* Creates application controller
* Creates QML engine

### CController

* Creates and provides access to
  * the list of known repositories
  * the list of open repositories
  * the current repository
  * the Git interactive rebase editing

### CRepository

* Represents a repository
* Creates and provides access to
  * the corresponding versioning system commands
  * the file system model
  * the filter for the file system model
  * the model for lines of output when executing a process
  * the name of the current branch
  * the repository status (normal, rebasing, merging, ...)
  * the list of branches
  * the log of the current branch
  * the diff of the selected file
  * the log of the selected file
* Exposes to QML / JS the versioning methods: stage, unstage, commit, push, ...

### CTreeFileModel

* Inherits QFileSystemModel
* Shows changed files as a tree
* Provides access to the repository files

### CTreeFileModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CTreeFileModel
* Enables filtering files on
  * file name
  * GIT status property

### CFlatFileModel

* Inherits QAbstractListModel
* Shows changed files as a list
* Provides access to the repository files

### CFlatFileModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CFlatFileModel
* Enables filtering files on
  * file name
  * GIT status property

### CRepoFile

* The properties of a repository file
  * status
  * staged / unstaged flag

### CCommands

* A (kind of) interface for a versioning system
* Makes the versioning system abstract

### CGitCommands

* Inherits CCommands
* Implements Git versioning commands

### CSvnCommands

* Inherits CCommands
* Implements SVN versioning commands

### CHGCommands

* Inherits CCommands
* Partially implements Mercurial versioning commands

### CLogModel

* A collection of log lines

### CLogModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CLogModel
* Enables filtering a log on
  * commit author
  * commit message

### CLogLine

* A line of log : commit id, date, author, message

### CDiffModel

* A collection of CDiffLine

### CDiffModelProxy

* Inherits QSortFilterProxyModel
* Encapsulates CDiffModel
* Enables filtering a diff

### CDiffLine

* A line of diff between two files

## Code summary

As of 2019-09-20:

```
     151 text files.
     150 unique files.                                          
      27 files ignored.

http://cloc.sourceforge.net v 1.64  T=0.33 s (370.7 files/s, 52317.1 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                             27           1422            661           4270
QML                             57            839            142           4094
TypeScript                       3             59              0           2794
C/C++ Header                    33            898            930           1293
Qt Project                       1             13             17             35
XML                              2              0              0             23
Bourne Shell                     1              0              6              6
-------------------------------------------------------------------------------
SUM:                           124           3231           1756          12515
-------------------------------------------------------------------------------
```

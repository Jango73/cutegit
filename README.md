# CuteGit

[![Build](https://github.com/Jango73/cutegit/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/Jango73/cutegit/actions/workflows/build.yml)

![alt text](https://github.com/Jango73/cutegit/blob/main/Media/cutegit.jpg)

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

## Features

* Open one or more repositories, clone from URL, remove repositories
* Refresh view
* Show the output of the executed commands

### Git and Gerrit

* Show repository status (clean, merge, rebase, ...) with ahead/behind counts
* Show repository files as a flat list or a tree, colored according to status, with filter and sorting
* Show the branches and tags
* Show the graphical commit graph with filter
* Show the reflog
* Show the selected file's log
* Show staged and unstaged diffs, stage/unstage whole files, hunks or single lines
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
* Update submodules

### Mercurial

* Show the branches
* Show repository files as a flat list or a tree, colored according to status
* Show the global graph with filter
* Show the selected file's log
* Show the working tree diff
* Mark files as staged / unstaged
* Commit, amend, branch from a commit
* Pull, push and force push

### Subversion

* Show repository files as a flat list or a tree, colored according to status
* Show the branches (trunk + branches folder)
* Show the global graph with filter
* Show the selected file's log
* Show the unstaged diffs in the selected item (folder or file)
* Schedule new files (add) and unschedule them
* Delete files
* Revert files
* Commit
* Update (pull)
* Switch current branch
* Branch and tag from a revision
* View the diff of a revision
* Blame

## Code summary

`CuteGit/sources` + `CuteGit/qml`: 146 files (35 C++, 38 C++ headers, 73 QML), 20402 lines (10025 C++, 3997 headers, 6380 QML).

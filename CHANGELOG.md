# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2026-10-25

### Added
- Subversion backend with capability-gated UI and test repository creation script.
- Multi-layout UI with layouts.json beside binary, embedded qrc fallback, layout choice in Options menu.
- Line-level staged/unstaged diff with patch builder, save diff as patch.
- Dedicated CommitDiff tab next to working-tree Diff tab, CommitDetails tab, collapse/expand per file plus collapse-all/expand-all buttons.
- Ahead/behind tracking state as colored log labels, single for-each-ref branch/tag batches with counts.
- Poll repositories for external changes with auto-refresh of current repository.
- Ignored files fetched on demand with toggle, persist View filters, untracked files shown by default with content in unstaged diff.
- Amend without ahead, force push, pull prioritized over push in magic button table.
- Submodule open from file menu with gitfile checkout detection, update submodules entry in local menu.
- Desktop shortcut script and application icon, dark theme by default, build timestamp on console at startup.
- Automated tests (diff, startup, repository) plus smoke runner, local CI script mirroring GitHub CI, GitHub CI workflow.
- CPack DEB and TGZ packaging, release procedure with CHANGELOG skeleton.

### Changed
- Port to Qt6 (folder dialogs via CController QFileDialog, QtQuick.Dialogs, QProcess splitCommand helper).
- Build migrated from qmake to CMake plus Ninja, harmonized scripts layout and CLI.
- Graph made lazy and unbounded with paginated loading, lanes drawn with rounded elbows.
- Log panes load on demand, file inventory seeded from VCS instead of crawling the folder.
- Startup scans repositories on first visible tab, FilePane simplified to plain Item without ExtendablePane.
- Status bar reports started/finished/failed for commit/amend/push/pull/fetch with process error summary.

### Fixed
- O(N2) freeze on repository file refresh, log flicker reconciled in place, stale untracked files after external commit.
- Stale file selection after revert, equal staged/unstaged split, DiffView segfault on collapsed file expand from stale cached model indexes.
- Branch and tag labels refreshed on background refresh and push, resync of working-tree diff after refresh preserving requested commit diffs.
- Git pull on diverged branches with --no-rebase, origin/HEAD symbolic reference excluded from remote branches.
- QML startup warnings, signal parameters, delegate widths, StandardTabView binding loop, SVG decoding errors.
- Extend/collapse losing pane layout on Qt6, scroll inertia and end bounce removed, Material buttons compacted vertically.
- Change commit message no-op (stub editor path, prefix id match, QML commit id scope).

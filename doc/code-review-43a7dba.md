# Code review of commit 43a7dba - Add descriptive multi-layout UI with JSON layouts beside binary

Commit: 43a7dba4a0bb614c38ddba737b68ea54439a2419
Date: Wed Oct 14 07:40:03 2026 +0200
Scope: CuteGit/CuteGit.pri, CuteGit/deploy/layouts.json, CuteGit/qml/App.qml, CuteGit/qml/components/Const.qml, CuteGit/qml/components/DockPanel.qml, CuteGit/qml/pages/MainMenu.qml, CuteGit/qml/tabs/*.qml (10 files), CuteGit/qml/views/RepositoryView.qml, CuteGit/resources.qrc, CuteGit/sources/CController.cpp, CuteGit/sources/CController.h, CuteGit/sources/CLayout.cpp, CuteGit/sources/CLayout.h, scripts/linux/build.sh, scripts/linux/package.sh

Issues sorted by decreasing severity.

## Functional bugs

1. `CuteGit/qml/tabs/BranchLogTab.qml:31`: `visible: root.repository === null | logView.count === 0` uses binary `|` instead of logical `||`. No short-circuit, implicit coercion.
2. `CuteGit/qml/views/RepositoryView.qml:100-102`: `onCurrentIndexChanged: { root.repository.flatFileModelProxy.handleCurrentIndex(currentIndex) }` without null guard. If the repository becomes null during an index change (close), null dereference / TypeError.
3. `CuteGit/qml/views/RepositoryView.qml:145,466-469`: `showBranchMenu` and `onRequestSwitchToBranch: root.repository.currentBranch = ...` without `root.repository !== null` guard and without `can(CEnums.SwitchBranch)` check. `BranchesTab.qml:43-47,89-93` checks the capability on double-click, the menu path bypasses it.
4. No validation that a layout covers the required tabs. A custom `layouts.json` without `Diff` / `Files` makes `activateFileDiffView()`, `activateTab()`, `getSelectedFiles()` silent: `RepositoryView.qml:512-518,500-510` returns nothing, `requestCommitDiffPrevious:419-422` believes it displayed the diff, `requestStageSelection:375-377` stages nothing without user feedback.
5. Silent functional duplicate: if a layout declares two `Files` panels, `getSelectedFiles()` returns only the first non-empty one. Partial staging without warning.
6. `RepositoryView.qml:116-134`: the `Repeater` destroys/recreates every `DockPanel` (hence every tab `Loader`) on each `currentLayoutPanels` change. Loss of state (selection, scroll, focus), expensive reload. `CController.cpp:274` makes it worse: `setCurrentLayoutPanels()` is called even when the index did not change.
7. `CuteGit/qml/tabs/BranchesTab.qml:34,80`: `model.name === root.repository.currentBranch` in both delegates without null guard. Evaluated with a null repository during model transitions.
8. `CuteGit/qml/components/DockPanel.qml:39`: `currentIndex: tabBar.currentIndex` is one-way. If `StandardTabView` allows swiping, `tabBar` does not follow. Tab/view desynchronization.
9. `CuteGit/qml/components/DockPanel.qml:20,45-46`: empty panel (`tabIds === []`) renders a blank pane without placeholder. `tabBar.currentIndex = 0` out of range, `Loader source: ""` silent.
10. `CuteGit/qml/tabs/BranchesTab.qml:97-99`: `focusTab()` always forces `remoteBranchList`, even when the remote list is empty/hidden. The local list never gets initial focus.

## Architecture, reuse, single source of truth

11. Dual source of truth: `CuteGit/deploy/layouts.json` exactly duplicates `CLayout::defaultLayouts()` (`CLayout.cpp:110-187`). Any change on one side diverges from the other. The fallback should be generated or the JSON embedded as a resource.
12. Stringly-typed C++/QML contract without shared constants: keys `panelId,posX,posY,width,height,tabs,tabId` (`CLayout.cpp:195-205`) read as `modelData.posX...modelData.tabs` (`RepositoryView.qml:123-132`, `DockPanel.qml:14,133,145`). A typo yields `undefined` / `NaN` geometry without error.
13. Tab vocabulary scattered across 5 places without a single reference: `CLayout::knownTabIds()` / `defaultLayouts()` / `layouts.json` / `DockPanel.tabTitle()` / `DockPanel.tabSource()` / `RepositoryView.activate*` / `findTab("Files")`. Adding one tab = 5 manual edits.
14. `CLayout.cpp:59`: `CLayout::knownTabIds().contains(sTabId)` rebuilds a `QStringList` for every tested tab. Allocations in a loop + unknown tabs filtered silently without logging the culprit.
15. `scripts/windows/build.bat` not updated: no copy of `layouts.json` next to the binary, unlike `scripts/linux/build.sh:125-127` and `scripts/linux/package.sh:41-43`. On Windows, systematic fallback to defaults with only `qWarning`. Moreover `CuteGit.pri:116,147` already lists `deploy/layouts.json` in `DEPLOY_NAMES`: redundant double deployment mechanism.
16. `SLayoutTab` (`CLayout.h:15-18`) defined but never used (`SLayoutPanel.lTabIds` is a `QStringList`). Dead code.
17. `reloadLayouts()` (`CController.cpp:279-296`) Q_INVOKABLE but never exposed in the menu, no auto-reload, error only `qWarning`, single `applicationDirPath` path without `qrc` / deploy folder fallback. Failure invisible to the user.

## Code duplication

18. `DockPanel.qml:64-110`: `tabTitle()` and `tabSource()` are two parallel 10-branch `if` chains over the same IDs. Same for `findTab/activateTab/getSelectedFiles` (`DockPanel.qml:131-163`) and `getSelectedFiles/activateTab` (`RepositoryView.qml:500-518`): same `for` loop over `panelRepeater` / `tabIds`.
19. `BranchesTab.qml:22-49,68-95`: remote/local delegates copied within 16 lines (`model`, `listView`). `RepositoryView.qml:473-498`: `requestRebaseOnBranch/requestMergeBranch/requestDeleteBranch` triplicate the `confirm.title/message/action` block. All 10 `qml/tabs/*Tab.qml` files repeat the 3 `repository/viewRoot/flatSelection` properties + `focusTab()`, even when unused (`GraphTab`, `TagsTab`, `RefLogTab`).
20. `CLayout.cpp:114-184`: construction of the 8 panels by repeated assignments (`sId/fPosX/fPosY/fWidth/fHeight/lTabIds`). `CController.cpp:257-275` / `279-296`: scattered clamp/emit logic. `build.sh` / `package.sh` duplicate the `cp layouts.json` with two syntaxes `[` vs `[[`.

## Conventions (AGENTS.md)

21. Declaration order: `Defines` section (`static const char*`) at the bottom of `CLayout.h:70-83` while macros/defines = group 1. Inconsistent separator `CLayout.cpp:46` (`//****`) vs `32,65,109,189,214` (`/****/`). Helpers `toPercent/toPanel` without doxygen header while the rule requires a header + 75-character separator per function.
22. Types: `const char*` for `sFileName/sTag*/sAttribute*` (`CLayout.h:74-83`) instead of Qt types (`QString`), unlike `sParam*` as `const QString` in `CController.cpp:58-70`. Parameter `loadLayouts(const QString& sFileName...)` (`CLayout.cpp:66`) shadowing the static member `CLayout::sFileName`: ambiguous naming. Mix of unqualified `sTagLayout` (`78`) vs qualified `CLayout::sAttributeId` (`50`).
23. Vocabulary / naming: abbreviations `Id` (Identifier), `Pos` (Position), `sParam/lNames/vLayouts/tLayout/xRoot/pErrorString`; `branchName` (`RepositoryView.qml:18`) vague and shared as mutable menu state (should be `menuBranchName`); `SLayoutPanel` in PascalCase while the rule requires SCREAMING_SNAKE_CASE for structs/defines.
24. `CController` constructors: the slave constructor (`CController.cpp:150-186`) initializes neither `m_bShowRenamed/m_bShowMissing/m_bShowIgnored` nor layouts (`reloadLayouts` missing), copied pattern without correction. Leading blank line `CLayout.h:1`, `CController.cpp:1`.

## Magic numbers and strings

25. Numbers: `/ 100` + `* 0.5` (`RepositoryView.qml:123-126`), clamps `0.0/100.0` and fallbacks `"0"/"100"` repeated 4x (`CLayout.cpp:51-54`), geometries `0/20/40/60/80/100` and widths `20/40/60` (`CLayout.cpp:117-182`, `layouts.json`), `parent.height * 0.5` (`BranchesTab.qml:26`), separator `height: 2` (`BranchesTab.qml:54`, should be `Const`), `(Const.elementHeight * 1.5) * 2` (`RepositoryView.qml:29`). Rule: hexadecimal except sizes/vectors/points/time, here without named constant.
26. Strings: IDs `"Files/Branches/Tags/BranchLog/Graph/RefLog/Diff/FileLog/Blame/Output"` (~10 occurrences per file), paths `"../tabs/*Tab.qml"` (`DockPanel.qml:89-108`), panel keys, `"layouts.json"` (`CLayout::sFileName` + 2 scripts + `pri`), `"Classic/BranchesLeft"`, `"No valid layout found in %1, using defaults"` (`CLayout.cpp:102`, written directly as `QString` without constant). Layout names displayed raw in the menu (`MainMenu.qml:452-469`) without `qsTr`, unlike `Const.layoutText`.

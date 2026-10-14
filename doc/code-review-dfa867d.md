# Code review of commit dfa867d - Staged unstaged line-level diff with patch builder

Commit: dfa867d94cd61b4bd4b4cbb0e5740783f17b9b4e
Date: Wed Oct 14 14:42:15 2026 +0200
Scope: CuteGit/CuteGit.pri, CuteGit/qml/components/Const.qml, CuteGit/qml/pages/DiffMenu.qml, CuteGit/qml/pages/FileDiffPane.qml, CuteGit/qml/tabs/DiffTab.qml, CuteGit/qml/views/DiffView.qml, CuteGit/qml/views/RepositoryView.qml, CuteGit/sources/CDiffLine.h, CuteGit/sources/CDiffModel.cpp, CuteGit/sources/CDiffModel.h, CuteGit/sources/CEnums.h, CuteGit/sources/CFlatFileModelProxy.cpp, CuteGit/sources/CPatchBuilder.cpp, CuteGit/sources/CPatchBuilder.h, CuteGit/sources/CRepository.cpp, CuteGit/sources/CRepository.h, CuteGit/sources/commands/CCommands.cpp, CuteGit/sources/commands/CCommands.h, CuteGit/sources/commands/CGitCommands.cpp, CuteGit/sources/commands/CGitCommands.h

Issues sorted by decreasing severity.

## Functional bugs

- [x] 1. `CuteGit/sources/commands/CGitCommands.cpp:941-943`: `handleDiffOutput` discards every line whose trimmed form is empty (`if (not sTrimmedLine.isEmpty())`). A context line for an empty file line has an empty body once the prefix is removed, so it is lost, the old/new counters shift and the rebuilt patch is corrupt.
- [x] 2. `CGitCommands.cpp:~970` + `CuteGit/sources/CPatchBuilder.cpp:97-124`: `iHunkId` resets to `-1` on each `diff` line, but `patchFromSelection` groups rows globally by `hunkId`. On a multi-file diff, hunk 0 of file A merges with hunk 0 of file B into one patch. Latent for the single-file staged/unstaged models, active for the multi-file commit model.
- [x] 3. `CPatchBuilder.cpp:196-227`: hunk start numbers are never recomputed after dropping unselected added lines, so later hunks keep stale starts. On regexp parse failure `iOldStart/iNewStart` stay silently `0`, which wrongly switches the patch sides to `/dev/null`.
- [x] 4. `CGitCommands.cpp:497-499`: `applyPatchNow` applies `Q_UNUSED(bCached)` and only uses the `--cached` commands. The signature promises an optional index-only mode that does not exist; a caller passing `false` believes it applies to the working tree.
- [x] 5. `CuteGit/sources/CRepository.cpp:907,1514-1530,515-518`: `requestWorkingTreeDiffs` fires two asynchronous commands without clearing the models or tagging the target file, and `onNewOutputListOfCDiffLine` fills the models without checking which file the result belongs to (race: the diff of file A overwrites file B). `saveFileDiffAsPatch` concatenates `staged + unstaged` in a fixed order independent of arrival order, without a guaranteed separator/newline.
- [x] 6. `CRepository.cpp:955-971`: the message "No staged change in selection" is also shown for an unstage operation (wrong verb), and both user-visible C++ strings bypass `tr()` while QML correctly uses `qsTr`.
- [x] 7. `CRepository.cpp:1333-1335`: `delete pNewFile; continue;` deletes the object but leaves the dangling pointer in the local copy of `lNewFiles`, reused afterwards via `containsKey`. No immediate crash (`containsKey` does not dereference), but manual ownership handling prone to double-free.

## Architecture, reuse, duplication

- [x] 8. `CPatchBuilder.cpp:13` vs `CGitCommands.cpp:98`: `PATCH_HUNK_REGEXP` duplicates `sDiffHunkRegExp` character for character, with a "mirrors" comment admitting the duplication. The pattern requires a single shared constant.
- [x] 9. `CuteGit/qml/views/DiffView.qml:14-125`: reimplements a parallel selection system (`selectedRows`, `selectionAnchor`, `selectOnly/toggleRow/selectRangeTo/selectAllRows`, `handleKeyboardMovement`, `onClicked`) while `StandardListView.qml:68-175` already provides `selection`, `handleKeyboardMovement`, `selectOnly/selectRange/selectAll`, `itemClicked`, `indexSelected`. `DiffView.handleKeyboardMovement` shadows the base one and breaks Up/Down/PageUp/PageDown/Home/End handling (no `changeCurrentIndex`, `previousIndex`, `positionViewAtIndex`); Ctrl+A is handled twice with conflicting semantics.
- [x] 10. `CRepository.cpp:676-730`: `stageDiffHunk`/`unstageDiffHunk` have identical bodies except for the model; the same holds for `stageDiffLines`/`unstageDiffLines` and for the duplicated `eStaged/eUnstagedFileDiff` branches of `onNewOutputString` and `onNewOutputListOfCDiffLine`. The no-duplicate-code rule requires one helper parameterized by (model, direction).
- [x] 11. `CGitCommands.cpp:944-1010`: `setFileName/setHunkId/setOldLineNumber/setNewLineNumber` repeated in every branch, with a redundant double set for `DiffFileName` (already set before the `if`). An intermediate construction function removes the duplication.
- [x] 12. `FileDiffPane.qml` (two near-identical `DiffView` blocks) and `DiffMenu.qml:48-91` (four near-identical stage/unstage line/hunk `Action` blocks): factorize through a shared component or parameterized actions.
- [x] 13. `CPatchBuilder.cpp:24-50`: `sourceRowsFromProxyRows` takes `pModel` without ever using it (no `Q_UNUSED` either): misleading signature, either drop the parameter or validate membership.
- [x] 14. `CRepository.cpp:700-730`: hunk staging works on direct source rows and bypasses the text filter, while line staging goes through the proxy: filter/non-filter inconsistency between the two paths.
- [x] 15. Dead code / dead plumbing: `DiffView.isSelected()` is defined but never used (the delegate repeats `indexOf` inline); `proxyRow` travels `DiffView -> FileDiffPane -> DiffTab -> RepositoryView -> DiffMenu` without ever being read.

## Conventions (AGENTS.md)

- [ ] 16. `CCommands.cpp:294-310`: `stagedFileDiff` and `applyPatchNow` lack a doxygen (`\fn`) header while the rule requires a header plus a 75-character separator per function (the neighbouring `CGitCommands` methods comply).
- [ ] 17. `CRepository.cpp:9`: the added `#include <QRegularExpression>` is unused in that file.
- [ ] 18. `CRepository.cpp` totals 1630 lines (limit: 1000) and this commit adds about 150 lines instead of splitting by responsibility.
- [ ] 19. `#define PATCH_HUNK_REGEXP "..."` (`CPatchBuilder.cpp:13`): a C macro for a string while the rule mandates Qt types; a `static const QString` also removes `QString(PATCH_HUNK_REGEXP)` at `:94`.
- [ ] 20. Prefixes: local `mHunkRows`/`mHunkHeaders` use `m_` (reserved for members, `CPatchBuilder.cpp:97-98`); `sSelectedSourceRows` is a `QSet` with the `s_` prefix of `QString` (`:88`).
- [ ] 21. Vocabulary / naming: `RegExp` abbreviation violates the full-words rule (pre-existing, this commit adds one more occurrence instead of correcting it); `isStagable` (`DiffMenu.qml:33`) misspells `isStageable`; `isGit` is true for Git or Gerrit (`FileDiffPane.qml:15-17`), a misleading name whose logic is duplicated between `FileDiffPane` and `DiffMenu.isStagable`.
- [ ] 22. `CRepository.cpp:515,1084,1092,1430,1516...`: direct reads of `m_bFileDiffFromCommit` in six places while it became a `Q_PROPERTY` with a setter used elsewhere; mixing `m_` access and accessor bypasses notification.
- [ ] 23. `CDiffModel.h/.cpp`: `lines()` returns the `QList` by value (O(n) copy on each `patchFromSelection` call) and exposes mutable model-owned pointers; return a constant reference.
- [ ] 24. `CRepository.h:5-8`: include order not alphabetical (`QStringListModel, QHash, QSet, QVariant`); `CGitCommands.h`: misaligned `sCommandApplyPatchCached   =` (`:91-92`).

## Magic numbers and strings

- [ ] 25. The `-1` sentinel is scattered without a named constant (`CDiffLine.h:41`, `DiffView.qml` anchor/row/hunk ids, `setOld/NewLineNumber(-1)`, `iHunkId < 0`); capture indices `captured(1/4/7)`, `mid(1)`, `index(..., 0)`, `preferredHeight: 2/1` (`FileDiffPane.qml`) all require named constants.
- [ ] 26. Magic strings: `"diff"`, `"@@"`, `"+"`, `"-"`, `"+++"/"---"`, `"index"`, `"\\"`, `" "`, `"@@ -%1,%2 +%3,%4 @@%5"`, `"diff --git a/%1 b/%1"`, `"--- %1"/"+++ %1"`, `"a/%1"/"b/%1"/"/dev/null"` (`CPatchBuilder.cpp:168-237`), `"No staged change..."/"Unable to create patch file"/"CuteGitStageXXXXXX.patch"` (`CRepository.cpp`), `"▶ "/"▼ "` (`DiffView.qml`), `"#80404040"` (`Const.qml`). `-U3` and `--unidiff-zero` (`CGitCommands.cpp:89-92`) ship without a comment explaining the `3` or the need for `zero`.
- [ ] 27. Redundant `QString("...")` wrappers instead of direct literals: `QString(" ") + sText.mid(1)` (`:185`), `QString("/dev/null")`, `QString("a/%1")` (`:231-232`).
- [ ] 28. `FileDiffPane.qml`: `Const.unstagedDiffText + " (" + count + ")"` concatenates `" ("`/`")"` instead of a `qsTr` template with `%1`, untranslatable and RTL-incorrect.
- [ ] 29. `DiffView.qml:121-131`: `Connections { target: root.model }` with a potentially `undefined` model (null repository); the global `Keys.onPressed` Ctrl+A handler also intercepts input while the filter field has focus.

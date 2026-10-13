# Third-party code issues

This document records defects and design constraints found in third-party code used by this project, with the evidence backing each claim.

## Qt 6.2.4: dependent anchors miss geometry updates on `ParentChange` revert

### Affected project code

- `CuteGit/qml/components/ExtendablePane.qml`: the `extended` state combines a `ParentChange` (reparent to `extensionParent`) with `AnchorChanges` (fill `extensionParent`).
- Users of the state: `FilePane`, `ToolPane`, `BranchLogPane`, `OutputPane`, all hosted in `CuteGit/qml/views/RepositoryView.qml` with `extensionParent: root`.

### Symptom

Extend a pane (`^` button), then collapse it (`v` button). The pane itself returns to its docked size, but its content keeps the extended size and paints outside the pane, under the sibling pane. The layout never recovers, even after resizing the window.

### Evidence (minimal QML reproduction, offscreen, Qt 6.2.4)

Pane `width: parent.width * 0.5`, inner item `anchors.fill: parent`:

| Stage    | Outer | Inner content |
|----------|-------|---------------|
| Docked   | 400   | 388           |
| Extended | 1000  | 988           |
| Restored | 400   | 988 (stuck)   |

Bisection results:

- A plain `Item` hierarchy with no Controls 2 types shows the same stuck inner size. The defect sits in Qt Quick core, not in Controls 2 `Pane` or its `contentItem`.
- The same state using `AnchorChanges` only, without `ParentChange`, restores inner size exactly (388). `ParentChange` is the trigger.
- After a stuck restore, enlarging the window leaves the outer width at 400 instead of the bound value 500. The revert replaced the `width: parent.width * 0.5` binding with an explicit constant.
- The inner anchors are still live after the stuck restore: changing the pane `padding` one frame later re-syncs the content (988 to 386). Exactly one dependent update, the revert resize, is lost. A synchronous nudge inside `onStateChanged` does not re-sync it.

### Design versus defect

Two separate phenomena share this symptom and must not be confused:

1. Binding replacement on the reparented item: documented design. `ParentChange` reparents an item while preserving its visual appearance, which requires assigning explicit `x`, `y`, `width` and `height` values. In QML, an explicit assignment replaces any binding. This behavior is described in the Qt `ParentChange` documentation and predates Qt 6.
2. Lost dependent anchor update on revert: defect. No documentation states that children or siblings anchored to the reparented item stop tracking its geometry. The extended resize reaches the dependents, the revert resize does not, while the item itself carries the correct value.

The visible damage comes from point 2. Point 1 makes it permanent: with the outer width frozen to a constant, no later window resize heals the layout.

### Regression status

No Qt 5 runtime is available in this environment, so a Qt 5 versus Qt 6 comparison of point 2 was not performed here. Point 1 exists in Qt 5 as well; any behavioral difference between the versions lies in point 2.

### Applied structural fix

The panes are hosted at the fullscreen host from the start and switch between docked and extended geometry with `AnchorChanges` only (no `ParentChange`). The docked geometry is expressed with anchors to the `centralPart` and `bottomPart` layout guides, which stay nested in `RepositoryView`. This avoids the defective revert path instead of forcing updates through it. The same arrangement restores outer and inner sizes exactly in the reproduction setup.

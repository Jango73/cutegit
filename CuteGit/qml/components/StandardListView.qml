import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQml.Models
import "."

// This item is complex because it implements desktop behavior
// i.e. navigating with the keyboard

ListView {
    id: root
    clip: true
    activeFocusOnTab: true
    contentWidth: width - scrollBarWidth
    flickDeceleration: 5000

    /*! If true, the items can be selected */
    property bool itemsSelectable: true

    /*! The selection model, if any */
    property variant selection: null

    /*! If true, the list will automatically scroll to end when items are added */
    property bool autoScrollToEnd: false

    /*! The size of the scroll bar */
    property int scrollBarWidth: Const.mainPadding * 2

    /*! If true, list is empty */
    readonly property bool empty: count === 0

    /*! For internal use */
    property variant modelIndices: ({})

    /*! For internal use */
    property int previousIndex: -1

    /*! Signals */
    signal spacePressed()
    signal enterPressed()
    signal deletePressed()

    /*! This timer positions the view at end */
    Timer {
        id: positionTimer
        interval: 100
        repeat: false
        onTriggered: root.positionViewAtIndex(root.count - 1, ListView.Beginning)
    }

    FocusIndicator {
        id: focusIndicator
        anchors.fill: parent
        visible: root.activeFocus && (root.itemsSelectable === false || root.count === 0)
    }

    ScrollBar.vertical: ScrollBar {
        width: scrollBarWidth
        active: true;
        minimumSize: (Const.mainPadding * 2) / height

        onActiveChanged: {
            if (!active)
                active = true;
        }
    }

    onCountChanged: {
        modelChanged()
    }

    onCurrentIndexChanged: {
        if (selection && currentIndex !== -1 && modelIndices[currentIndex])
            selection.setCurrentIndex(modelIndices[currentIndex], ItemSelectionModel.Current)

        if (!itemsSelectable)
            positionViewAtIndex(currentIndex, ListView.Beginning)
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Up) {
            handleKeyboardMovement(event, currentIndex - 1)
            event.accepted = true
        }
        else if (event.key === Qt.Key_Down) {
            handleKeyboardMovement(event, currentIndex + 1)
            event.accepted = true
        }
        else if (event.key === Qt.Key_PageUp) {
            handleKeyboardMovement(event, currentIndex - 5)
            event.accepted = true
        }
        else if (event.key === Qt.Key_PageDown) {
            handleKeyboardMovement(event, currentIndex + 5)
            event.accepted = true
        }
        else if (event.key === Qt.Key_Home) {
            handleKeyboardMovement(event, 0)
            positionViewAtIndex(currentIndex, ListView.Beginning)
            event.accepted = true
        }
        else if (event.key === Qt.Key_End) {
            handleKeyboardMovement(event, count - 1)
            positionViewAtIndex(currentIndex, ListView.End)
            event.accepted = true
        }
        else if (event.key === Qt.Key_Space) {
            spacePressed()
            event.accepted = true
        }
        else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            enterPressed()
            event.accepted = true
        }
        else if (event.key === Qt.Key_Delete) {
            deletePressed()
            event.accepted = true
        }
        else if (event.key === Qt.Key_A) {
            if (event.modifiers & Qt.ControlModifier) {
                if (selection)
                    selectAll()
            }
        }
    }

    function modelChanged() {
        if (autoScrollToEnd)
            positionTimer.start()

        if (count > 0 && currentIndex == -1) {
            previousIndex = -1
            currentIndex = 0
        }
    }

    /*! Returns \c true if the item at index is selected */
    function indexSelected(index) {
        return selection
                ? selection.hasSelection && selection.isSelected(modelIndices[index])
                : false
    }

    /*! Sets a model index for the specified integer index */
    function updateModelIndex(index) {
        modelIndices[index] = model.index(index, 0)
    }

    /*! Selects items based on keyboard movements */
    function handleKeyboardMovement(event, targetIndex) {
        if (event.modifiers & Qt.ShiftModifier) {
            changeCurrentIndex(targetIndex, true)

            if (selection) {
                selectTo(currentIndex)
            }
        } else {
            changeCurrentIndex(targetIndex, false)

            if (selection) {
                selectOnly(currentIndex)
            }
        }
    }

    /*! Sets the current index based on the state of the shift key */
    function changeCurrentIndex(index, shiftModifier) {
        if (shiftModifier) {
            if (previousIndex === -1)
                previousIndex = currentIndex
        } else {
            previousIndex = -1
        }

        currentIndex = index

        if (currentIndex < 0)
            currentIndex = 0
        if (currentIndex > count - 1)
            currentIndex = count - 1
    }

    function itemClicked(mouse, index, previousIndex) {
        if (mouse.button === Qt.RightButton) {
            if (indexSelected(index)) {
                return
            }
        }

        root.previousIndex = -1

        if (mouse.modifiers & Qt.ShiftModifier) {
            selectRange(previousIndex, index)
        } else {
            selectOnly(index)
        }
    }

    function selectTo(targetIndex) {
        if (selection) {
            selection.clear()

            if (previousIndex === -1) {
                if (targetIndex === currentIndex) {
                    selection.select(modelIndices[targetIndex], ItemSelectionModel.ToggleCurrent)
                } else {
                    selectRange(currentIndex, targetIndex)
                }
            } else {
                if (targetIndex === previousIndex) {
                    selection.select(modelIndices[targetIndex], ItemSelectionModel.ToggleCurrent)
                } else {
                    selectRange(previousIndex, targetIndex)
                }
            }
        }
    }

    function selectOnly(targetIndex) {
        selection.clear()
        selection.select(modelIndices[targetIndex], ItemSelectionModel.SelectCurrent)
    }

    /*! Selects a range of items */
    function selectRange(from, to) {
        var selIndex

        selection.clear()

        if (to >= from) {
            for (selIndex = from; selIndex <= to; selIndex++) {
                selection.select(modelIndices[selIndex], ItemSelectionModel.Select)
            }
        } else {
            for (selIndex = from; selIndex >= to; selIndex--) {
                selection.select(modelIndices[selIndex], ItemSelectionModel.Select)
            }
        }
    }

    function selectAll() {
        selectRange(0, count - 1)
    }
}

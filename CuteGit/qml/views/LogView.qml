import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "../components"

StandardListView {
    id: root

    /*! If true, a graph symbol from the model is shown before the text */
    property bool hasSymbol: false

    /*! The currently selected commit */
    property string currentCommitId: ""

    /*! The id of the diff 'from' commit */
    property string diffFromCommitId: ""

    /*! The id of the diff 'to' commit */
    property string diffToCommitId: ""

    signal itemClicked(var commitId)
    signal itemRightClicked(var commitId, var message)
    signal requestCommitDiffPrevious(var commitId)
    signal requestCommitDiffFrom(var commitId)
    signal requestCommitDiffTo(var commitId)

    function isValidDate(d) {
        return d instanceof Date && !isNaN(d);
    }

    delegate: StandardListViewItem {
        id: delegateItem
        objectName: "root.delegate." + index
        width: ListView.view.width
        expanded: true
        dataLoading: !model.messageIsComplete
        listView: root
        symbolText: ""
        graphVisible: root.hasSymbol
        graphCommitLane: model.graphCommitLane !== undefined ? model.graphCommitLane : -1
        graphVerticals: model.graphVerticals !== undefined ? model.graphVerticals : []
        graphConnections: model.graphConnections !== undefined ? model.graphConnections : []
        primaryText: fullText
        secondaryText: (model.commitId === "" || model.isWorkingTree === true) ? "" : model.author + " - " + (isValidDate(model.date) ? model.date : "")
        selectionShown: index === root.currentIndex
        focusShown: root.activeFocus && index === root.currentIndex

        property string commitId: model.commitId
        property variant labels: model.labels
        property string fullText: (
                                      model.markedAsDiffFrom
                                      ? Const.logLabelFrom
                                      : model.markedAsDiffTo
                                        ? Const.logLabelTo
                                        : ""
                                      ) + model.message

        secondaryZone: [
            // The row containing labels
            Row {
                id: labelLayout
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.rightMargin: Const.mainPadding
                spacing: Const.mainPadding
                width: implicitWidth

                Repeater {
                    id: labelsRepeater

                    model: delegateItem.labels

                    LogLabel {
                        x: Const.mainPadding
                        height: labelLayout.height
                        text: model.name
                        type: model.type
                    }
                }
            }
        ]

        onClicked: function(mouse, previousIndex) {
            if (mouse.button === Qt.RightButton) {
                root.itemRightClicked(model.commitId, model.fullMessage)
            } else if (mouse.button === Qt.LeftButton) {
                // Selection change already emitted itemClicked via handleItemChange:
                // only re-emit when clicking the already current (mono-line) item.
                if (index === previousIndex && !(mouse.modifiers & Qt.ShiftModifier)) {
                    root.itemClicked(model.commitId)
                }
            }
        }

        onDoubleClicked: root.requestCommitDiffPrevious(root.currentCommitId)
    }

    onCountChanged: updateCurrentCommitId()
    onCurrentIndexChanged: handleItemChange()

    onSpacePressed: root.requestCommitDiffPrevious(root.currentCommitId)

    onEnterPressed: {
        if (root.diffFromCommitId === "") {
            root.requestCommitDiffFrom(root.currentCommitId)
        } else {
            root.requestCommitDiffTo(root.currentCommitId)
        }
    }

    function updateCurrentCommitId() {
        if (root.currentItem)
            root.currentCommitId = root.currentItem.commitId
        else
            root.currentCommitId = ""
    }

    function handleItemChange() {
        updateCurrentCommitId()

        // Keyboard selection change acts like a click on the item,
        // but only for a mono-line selection (no Shift range anchor).
        if (root.previousIndex === -1 && root.currentCommitId !== "")
            root.itemClicked(root.currentCommitId)
    }
}

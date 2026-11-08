import QtQuick 2.12
import QtQuick.Controls 2.5

//-------------------------------------------------------------------------------------------------

// A generic horizontally scrollable strip. It is completely agnostic of its
// children: it only shows left/right scroll buttons when the content does not
// fit, and scrolls the content horizontally by a fixed step.
Item {
    id: root

    // Current index, kept for callers that use the strip as a tab bar.
    property int currentIndex: 0

    // Amount scrolled by each button press.
    property int scrollStep: 120

    // Children are placed inside the scrolling content row.
    default property alias contentChildren: contentRow.children

    implicitHeight: contentRow.implicitHeight

    //-------------------------------------------------------------------------------------------------

    readonly property bool overflow: contentRow.width > root.width + 1

    //-------------------------------------------------------------------------------------------------

    Row {
        id: layout
        anchors.fill: parent
        spacing: 0

        ToolButton {
            id: leftButton
            visible: root.overflow
            width: visible ? implicitWidth : 0
            text: "<"

            onClicked: {
                contentFlick.contentX = Math.max(0, contentFlick.contentX - root.scrollStep)
            }
        }

        Flickable {
            id: contentFlick
            width: parent.width - leftButton.width - rightButton.width
            height: parent.height
            contentWidth: contentRow.width
            contentHeight: height
            clip: true
            interactive: false
            boundsBehavior: Flickable.StopAtBounds

            Row {
                id: contentRow
                height: parent.height
                spacing: 0
            }
        }

        ToolButton {
            id: rightButton
            visible: root.overflow
            width: visible ? implicitWidth : 0
            text: ">"

            onClicked: {
                var maximumX = Math.max(0, contentFlick.contentWidth - contentFlick.width)
                contentFlick.contentX = Math.min(maximumX, contentFlick.contentX + root.scrollStep)
            }
        }
    }
}